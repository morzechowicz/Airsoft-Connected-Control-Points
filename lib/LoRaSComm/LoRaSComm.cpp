/*
 * LoRaSComm - Implementation
 */

#include "LoRaSComm.h"

// Constructor
LoRaSComm::LoRaSComm(SX1278 *radioModule)
    : radio(radioModule),
      myAddress(0),
      currentSequence(0),
      maxRetries(LORASCOMM_DEFAULT_MAX_RETRIES),
      ackTimeout(LORASCOMM_DEFAULT_ACK_TIMEOUT),
      initialized(false),
      rxQueue(nullptr),
      txQueue(nullptr),
      ackPendingQueue(nullptr),
      radioMutex(nullptr),
      rxTaskHandle(nullptr),
      txTaskHandle(nullptr),
      userTaskHandle(nullptr),
      txPacketCount(0),
      rxPacketCount(0),
      failedTxCount(0),
      lastRssi(0),
      lastSnr(0.0f),
      receiveCallback(nullptr)
{
}

// Destructor
LoRaSComm::~LoRaSComm()
{
    end();
}

bool LoRaSComm::begin(uint8_t address,
                      float frequency,
                      int8_t power,
                      uint8_t spreadingFactor,
                      float signalBandwidth,
                      uint8_t codingRate)
{

    // Validate address
    if (!LoRaSCommPacketCodec::isValidAddress(address) || address == LORASCOMM_BROADCAST_ADDR)
    {
        LORASCOMM_PRINTLN(F("[LoRaSComm] Error: Invalid address (must be 0x01-0xFE)"));
        return false;
    }

    myAddress = address;

    // Initialize RadioLib
    LORASCOMM_PRINT(F("[LoRaSComm] Initializing SX1278... "));
    int state = radio->begin(frequency,
                             signalBandwidth,
                             spreadingFactor,
                             codingRate,
                             0x12,
                             power,
                             8,  // preamble length
                             0); // gain (0 = auto)

    if (state != RADIOLIB_ERR_NONE)
    {
        LORASCOMM_PRINT(F("failed, code "));
        LORASCOMM_PRINTLN(state);
        return false;
    }
    LORASCOMM_PRINTLN(F("success!"));

    // Configure for explicit header mode (we need payload length)
    state = radio->explicitHeader();
    if (state != RADIOLIB_ERR_NONE)
    {
        LORASCOMM_PRINT(F("[LoRaSComm] Error: Failed to set explicit header mode, code "));
        LORASCOMM_PRINTLN(state);
        return false;
    }

    // Enable CRC in LoRa (hardware CRC, in addition to our packet CRC)
    state = radio->setCRC(true);
    if (state != RADIOLIB_ERR_NONE)
    {
        LORASCOMM_PRINT(F("[LoRaSComm] Warning: Failed to enable hardware CRC, code "));
        LORASCOMM_PRINTLN(state);
        // Not fatal, we have our own CRC
    }

    // Create FreeRTOS objects
    LORASCOMM_PRINTLN(F("[LoRaSComm] Creating FreeRTOS objects..."));

    rxQueue = xQueueCreate(LORASCOMM_RX_QUEUE_SIZE, sizeof(ReceivedPacket));
    if (rxQueue == nullptr)
    {
        LORASCOMM_PRINTLN(F("[LoRaSComm] Error: Failed to create RX queue"));
        return false;
    }

    txQueue = xQueueCreate(LORASCOMM_TX_QUEUE_SIZE, sizeof(TxQueueItem));
    if (txQueue == nullptr)
    {
        LORASCOMM_PRINTLN(F("[LoRaSComm] Error: Failed to create TX queue"));
        vQueueDelete(rxQueue);
        return false;
    }

    ackPendingQueue = xQueueCreate(LORASCOMM_ACK_PENDING_SIZE, sizeof(PendingAck));
    if (ackPendingQueue == nullptr)
    {
        LORASCOMM_PRINTLN(F("[LoRaSComm] Error: Failed to create ACK pending queue"));
        vQueueDelete(rxQueue);
        vQueueDelete(txQueue);
        return false;
    }

    radioMutex = xSemaphoreCreateMutex();
    if (radioMutex == nullptr)
    {
        LORASCOMM_PRINTLN(F("[LoRaSComm] Error: Failed to create radio mutex"));
        vQueueDelete(rxQueue);
        vQueueDelete(txQueue);
        vQueueDelete(ackPendingQueue);
        return false;
    }

    // Create FreeRTOS tasks
    LORASCOMM_PRINTLN(F("[LoRaSComm] Creating FreeRTOS tasks..."));

    BaseType_t result;

    // RX Task (highest priority)
    result = xTaskCreate(
        rxTask,
        "LoRaSComm_RX",
        LORASCOMM_RX_TASK_STACK,
        this, // Pass 'this' pointer as parameter
        LORASCOMM_RX_TASK_PRIORITY,
        &rxTaskHandle);

    if (result != pdPASS)
    {
        LORASCOMM_PRINTLN(F("[LoRaSComm] Error: Failed to create RX task"));
        end();
        return false;
    }

    // TX Task
    result = xTaskCreate(
        txTask,
        "LoRaSComm_TX",
        LORASCOMM_TX_TASK_STACK,
        this,
        LORASCOMM_TX_TASK_PRIORITY,
        &txTaskHandle);

    if (result != pdPASS)
    {
        LORASCOMM_PRINTLN(F("[LoRaSComm] Error: Failed to create TX task"));
        end();
        return false;
    }

    // User Task (for callbacks)
    result = xTaskCreate(
        userTask,
        "LoRaSComm_User",
        LORASCOMM_USER_TASK_STACK,
        this,
        LORASCOMM_USER_TASK_PRIORITY,
        &userTaskHandle);

    if (result != pdPASS)
    {
        LORASCOMM_PRINTLN(F("[LoRaSComm] Error: Failed to create User task"));
        end();
        return false;
    }

    initialized = true;

    LORASCOMM_LOG("[LoRaSComm] Initialized successfully! Address: 0x%02X, Frequency: %.1f MHz\n",
                  myAddress, frequency);

    return true;
}

void LoRaSComm::end()
{
    if (!initialized)
    {
        return;
    }

    LORASCOMM_PRINTLN(F("[LoRaSComm] Shutting down..."));

    // Delete tasks
    if (rxTaskHandle != nullptr)
    {
        vTaskDelete(rxTaskHandle);
        rxTaskHandle = nullptr;
    }

    if (txTaskHandle != nullptr)
    {
        vTaskDelete(txTaskHandle);
        txTaskHandle = nullptr;
    }

    if (userTaskHandle != nullptr)
    {
        vTaskDelete(userTaskHandle);
        userTaskHandle = nullptr;
    }

    // Delete queues and mutex
    if (rxQueue != nullptr)
    {
        vQueueDelete(rxQueue);
        rxQueue = nullptr;
    }

    if (txQueue != nullptr)
    {
        vQueueDelete(txQueue);
        txQueue = nullptr;
    }

    if (ackPendingQueue != nullptr)
    {
        vQueueDelete(ackPendingQueue);
        ackPendingQueue = nullptr;
    }

    if (radioMutex != nullptr)
    {
        vSemaphoreDelete(radioMutex);
        radioMutex = nullptr;
    }

    initialized = false;
    LORASCOMM_PRINTLN(F("[LoRaSComm] Shutdown complete"));
}

// Configuration methods
void LoRaSComm::setMaxRetries(uint8_t retries)
{
    maxRetries = retries;
}

void LoRaSComm::setAckTimeout(uint32_t ms)
{
    ackTimeout = ms;
}

void LoRaSComm::setAddress(uint8_t addr)
{
    if (LoRaSCommPacketCodec::isValidAddress(addr) && addr != LORASCOMM_BROADCAST_ADDR)
    {
        myAddress = addr;
    }
}

void LoRaSComm::onReceive(void (*callback)(const ReceivedPacket &packet))
{
    receiveCallback = callback;
}

// ========== PUBLIC API METHODS ==========

bool LoRaSComm::sendUnreliable(uint8_t dest, const uint8_t *data, size_t len)
{
    if (!initialized)
    {
        LORASCOMM_PRINTLN(F("[API] Error: Not initialized"));
        return false;
    }

    if (len > LORASCOMM_MAX_PAYLOAD_SIZE)
    {
        LORASCOMM_PRINTLN(F("[API] Error: Payload too large"));
        return false;
    }

    if (!LoRaSCommPacketCodec::isValidAddress(dest))
    {
        LORASCOMM_PRINTLN(F("[API] Error: Invalid destination address"));
        return false;
    }

    TxQueueItem item;
    item.destAddr = dest;
    item.dataLen = len;
    item.packetType = PACKET_DATA_UNRELIABLE;
    memcpy(item.data, data, len);

    if (xQueueSend(txQueue, &item, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        LORASCOMM_PRINTLN(F("[API] Error: TX queue full"));
        return false;
    }

    return true;
}

bool LoRaSComm::sendReliable(uint8_t dest, const uint8_t *data, size_t len, EventGroupHandle_t eventGroup, EventBits_t eventBit)
{
    if (!initialized)
    {
        LORASCOMM_PRINTLN(F("[API] Error: Not initialized"));
        return false;
    }

    if (len > LORASCOMM_MAX_PAYLOAD_SIZE)
    {
        LORASCOMM_PRINTLN(F("[API] Error: Payload too large"));
        return false;
    }

    if (!LoRaSCommPacketCodec::isValidAddress(dest))
    {
        LORASCOMM_PRINTLN(F("[API] Error: Invalid destination address"));
        return false;
    }

    if (dest == LORASCOMM_BROADCAST_ADDR)
    {
        LORASCOMM_PRINTLN(F("[API] Error: Cannot send reliable to broadcast address"));
        return false;
    }

    TxQueueItem item;
    item.destAddr = dest;
    item.dataLen = len;
    item.packetType = PACKET_DATA_RELIABLE;
    memcpy(item.data, data, len);
    item.eventGroup = eventGroup;
    item.eventBit = eventBit;

    if (xQueueSend(txQueue, &item, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        LORASCOMM_PRINTLN(F("[API] Error: TX queue full"));
        return false;
    }

    return true;
}

bool LoRaSComm::testSendAck(uint8_t dest)
{
    if (!initialized)
    {
        LORASCOMM_PRINTLN(F("[API] Error: Not initialized"));
        return false;
    }

    if (!LoRaSCommPacketCodec::isValidAddress(dest))
    {
        LORASCOMM_PRINTLN(F("[API] Error: Invalid destination address"));
        return false;
    }

    if (dest == LORASCOMM_BROADCAST_ADDR)
    {
        LORASCOMM_PRINTLN(F("[API] Error: Cannot send reliable to broadcast address"));
        return false;
    }

    sendAck(dest, 0); // Using 0 as test sequence number

    return true;
}

bool LoRaSComm::available()
{
    if (!initialized)
    {
        return false;
    }

    return uxQueueMessagesWaiting(rxQueue) > 0;
}

bool LoRaSComm::receive(ReceivedPacket &packet)
{
    if (!initialized)
    {
        return false;
    }

    return xQueueReceive(rxQueue, &packet, 0) == pdTRUE;
}

// ========== RX TASK ==========
void LoRaSComm::rxTask(void *params)
{
    LoRaSComm *instance = static_cast<LoRaSComm *>(params);

    LORASCOMM_PRINTLN(F("[RX Task] Started"));

    uint8_t buffer[LORASCOMM_MAX_PACKET_SIZE];

    // Put radio in RX mode initially
    if (xSemaphoreTake(instance->radioMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        instance->radio->startReceive();
        xSemaphoreGive(instance->radioMutex);
        LORASCOMM_PRINTLN(F("[RX Task] Radio in RX mode"));
    }

    while (true)
    {
        // Check if we can access the radio
        if (xSemaphoreTake(instance->radioMutex, pdMS_TO_TICKS(50)) == pdTRUE)
        {

            // Check if a packet is available (non-blocking)
            // We check the IRQ flags to see if RX is done
            uint16_t irqFlags = instance->radio->getIRQFlags();

            if (irqFlags & RADIOLIB_SX127X_CLEAR_IRQ_FLAG_RX_DONE)
            {
                // Packet received! Read it
                size_t len = instance->radio->getPacketLength();

                LoRaSCommPacket decodedPacket; // will hold decoded packet if successful
                bool haveDecoded = false;

                if (len > 0 && len <= LORASCOMM_MAX_PACKET_SIZE)
                {
                    int state = instance->radio->readData(buffer, len);

                    if (state == RADIOLIB_ERR_NONE)
                    {
                        // Get RSSI and SNR
                        instance->lastRssi = instance->radio->getRSSI();
                        instance->lastSnr = instance->radio->getSNR();

                        LORASCOMM_LOG("[RX Task] Raw packet received, len=%d, RSSI=%d\n", len, instance->lastRssi);

                        // Decode packet
                        LoRaSCommPacket packet;
                        if (LoRaSCommPacketCodec::decode(buffer, len, packet))
                        {
                            // Valid packet! store for processing after releasing radio
                            decodedPacket = packet;
                            haveDecoded = true;
                        }
                        else
                        {
                            LORASCOMM_PRINTLN(F("[RX Task] CRC validation failed"));
                            LORASCOMM_PRINT(F("[RX Task] Raw data: "));
                            for (size_t i = 0; i < len && i < 32; i++)
                            {
                                LORASCOMM_LOG("%02X ", buffer[i]);
                            }
                            LORASCOMM_PRINTLN();
                        }
                    }
                }

                // Clear IRQ flags and restart receive while we still have the radio mutex
                instance->radio->clearIrqFlags(irqFlags);
                instance->radio->startReceive();

                // Release radio mutex BEFORE handling the packet (so handleReceivedPacket can transmit ACK)
                xSemaphoreGive(instance->radioMutex);

                // Now process the decoded packet outside of the radio mutex
                if (haveDecoded)
                {
                    instance->handleReceivedPacket(decodedPacket);
                }

                // continue to next loop iteration (mutex already released)
                continue;
            }

            xSemaphoreGive(instance->radioMutex);
        }

        // Small delay to prevent task starvation
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ========== TX TASK ==========
void LoRaSComm::txTask(void *params)
{
    LoRaSComm *instance = static_cast<LoRaSComm *>(params);

    LORASCOMM_PRINTLN(F("[TX Task] Started"));

    TxQueueItem txItem;
    uint8_t buffer[LORASCOMM_MAX_PACKET_SIZE];

    while (true)
    {
        // Check for packets to send
        if (xQueueReceive(instance->txQueue, &txItem, pdMS_TO_TICKS(50)) == pdTRUE)
        {

            // Determine packet type
            LoRaSCommPacketType type = txItem.packetType;

            // Get sequence number
            uint8_t sequence = instance->currentSequence++;

            // Encode packet
            size_t packetSize = LoRaSCommPacketCodec::encode(
                buffer,
                instance->myAddress,
                txItem.destAddr,
                type,
                sequence,
                txItem.data,
                txItem.dataLen);

            if (packetSize > 0)
            {
                // Transmit the packet
                bool success = instance->transmitPacket(buffer, packetSize);

                if (success)
                {
                    instance->txPacketCount++;

                    // If reliable, add to pending ACK queue
                    if (txItem.packetType == PACKET_DATA_RELIABLE)
                    {
                        PendingAck pending;
                        pending.destAddr = txItem.destAddr;
                        pending.sequence = sequence;
                        pending.retryCount = 0;
                        pending.sentTime = millis();
                        pending.dataLen = txItem.dataLen;
                        memcpy(pending.data, txItem.data, txItem.dataLen);
                        pending.active = true;

                        // Add to pending queue (non-blocking)
                        if (xQueueSend(instance->ackPendingQueue, &pending, 0) != pdTRUE)
                        {
                            LORASCOMM_PRINTLN(F("[TX Task] Warning: ACK pending queue full"));
                            instance->failedTxCount++;
                        }
                    }

                    LORASCOMM_LOG("[TX Task] Sent %s packet to 0x%02X, seq=%d, size=%d\n",
                                  txItem.packetType == PACKET_DATA_RELIABLE ? "RELIABLE" : "UNRELIABLE",
                                  txItem.destAddr, sequence, packetSize);
                }
                else
                {
                    LORASCOMM_PRINTLN(F("[TX Task] Transmission failed"));
                    instance->failedTxCount++;
                }
            }
        }

        // Process ACK timeouts and retries
        instance->processAckTimeout();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ========== USER TASK ==========
void LoRaSComm::userTask(void *params)
{
    LoRaSComm *instance = static_cast<LoRaSComm *>(params);

    LORASCOMM_PRINTLN(F("[User Task] Started"));

    ReceivedPacket packet;

    while (true)
    {
        // Check for received packets to process
        if (xQueueReceive(instance->rxQueue, &packet, pdMS_TO_TICKS(100)) == pdTRUE)
        {

            // Call user callback if set
            if (instance->receiveCallback != nullptr)
            {
                instance->receiveCallback(packet);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ========== HELPER METHODS ==========

void LoRaSComm::handleReceivedPacket(const LoRaSCommPacket &packet)
{
    // Check if packet is for us or broadcast
    if (packet.header.destAddr != myAddress && packet.header.destAddr != LORASCOMM_BROADCAST_ADDR)
    {
        // Not for us, ignore
        return;
    }

    rxPacketCount++;

    switch (packet.header.packetType)
    {
    case PACKET_DATA_UNRELIABLE:
    case PACKET_DATA_RELIABLE:
    {
        // Send ACK if reliable
        if (packet.header.packetType == PACKET_DATA_RELIABLE &&
            packet.header.destAddr != LORASCOMM_BROADCAST_ADDR)
        {
            sendAck(packet.header.srcAddr, packet.header.sequence);
        }

        // Add to RX queue for user
        ReceivedPacket rxPacket;
        rxPacket.srcAddr = packet.header.srcAddr;
        rxPacket.dataLen = packet.header.payloadLen;
        memcpy(rxPacket.data, packet.payload, packet.header.payloadLen);
        rxPacket.rssi = lastRssi;
        rxPacket.snr = lastSnr;

        if (xQueueSend(rxQueue, &rxPacket, 0) != pdTRUE)
        {
            LORASCOMM_PRINTLN(F("[RX] Warning: RX queue full, packet dropped"));
        }

        LORASCOMM_LOG("[RX] Received %s from 0x%02X, seq=%d, len=%d, RSSI=%d\n",
                      packet.header.packetType == PACKET_DATA_RELIABLE ? "RELIABLE" : "UNRELIABLE",
                      packet.header.srcAddr, packet.header.sequence,
                      packet.header.payloadLen, lastRssi);
        break;
    }

    case PACKET_ACK:
    case PACKET_NACK:
    {
        // Remove from pending ACK queue
        PendingAck pending;
        UBaseType_t queueLength = uxQueueMessagesWaiting(ackPendingQueue);

        for (UBaseType_t i = 0; i < queueLength; i++)
        {
            if (xQueueReceive(ackPendingQueue, &pending, 0) == pdTRUE)
            {
                // Check if this ACK matches
                if (pending.destAddr == packet.header.srcAddr &&
                    pending.sequence == packet.header.sequence)
                {
                    // Match! Remove from queue
                    LORASCOMM_LOG("[RX] Received %s from 0x%02X for seq=%d\n",
                                  packet.header.packetType == PACKET_ACK ? "ACK" : "NACK",
                                  packet.header.srcAddr, packet.header.sequence);
                    // Don't put back in queue
                }
                else
                {
                    // Not a match, put back in queue
                    xQueueSend(ackPendingQueue, &pending, 0);
                }
            }
        }
        break;
    }

    default:
        LORASCOMM_LOG("[RX] Unknown packet type: 0x%02X\n", packet.header.packetType);
        break;
    }
}

void LoRaSComm::sendAck(uint8_t dest, uint8_t sequence)
{
    uint8_t buffer[LORASCOMM_MAX_PACKET_SIZE];
    size_t packetSize = LoRaSCommPacketCodec::encodeAck(buffer, myAddress, dest, sequence);

    if (packetSize > 0)
    {
        // Small delay before sending ACK to let sender switch to RX
        vTaskDelay(pdMS_TO_TICKS(100));

        transmitPacket(buffer, packetSize);
        LORASCOMM_LOG("[TX] Sent ACK to 0x%02X for seq=%d\n", dest, sequence);

        // Small delay after ACK to let it transmit
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void LoRaSComm::sendNack(uint8_t dest, uint8_t sequence)
{
    uint8_t buffer[LORASCOMM_MAX_PACKET_SIZE];
    size_t packetSize = LoRaSCommPacketCodec::encodeNack(buffer, myAddress, dest, sequence);

    if (packetSize > 0)
    {
        transmitPacket(buffer, packetSize);
        LORASCOMM_LOG("[TX] Sent NACK to 0x%02X for seq=%d\n", dest, sequence);
    }
}

bool LoRaSComm::transmitPacket(const uint8_t *buffer, size_t len)
{
    if (xSemaphoreTake(radioMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        int state = radio->transmit(const_cast<uint8_t *>(buffer), len);

        // Put radio back in RX mode after TX
        if (state == RADIOLIB_ERR_NONE)
        {
            // Small delay to ensure transmission completes
            vTaskDelay(pdMS_TO_TICKS(10));
            radio->startReceive();
        }
        else
        {
            Serial.println(state);
        }

        xSemaphoreGive(radioMutex);

        return (state == RADIOLIB_ERR_NONE);
    }

    return false;
}

void LoRaSComm::processAckTimeout()
{
    PendingAck pending;
    UBaseType_t queueLength = uxQueueMessagesWaiting(ackPendingQueue);

    for (UBaseType_t i = 0; i < queueLength; i++)
    {
        if (xQueueReceive(ackPendingQueue, &pending, 0) == pdTRUE)
        {

            if (!pending.active)
            {
                continue; // Skip inactive entries
            }

            uint32_t elapsed = millis() - pending.sentTime;

            if (elapsed >= ackTimeout)
            {
                // Timeout! Check if we should retry
                if (pending.retryCount < maxRetries)
                {
                    // Retry
                    pending.retryCount++;
                    pending.sentTime = millis();

                    // Re-encode and send
                    uint8_t buffer[LORASCOMM_MAX_PACKET_SIZE];
                    size_t packetSize = LoRaSCommPacketCodec::encode(
                        buffer,
                        myAddress,
                        pending.destAddr,
                        PACKET_DATA_RELIABLE,
                        pending.sequence,
                        pending.data,
                        pending.dataLen);

                    if (transmitPacket(buffer, packetSize))
                    {
                        LORASCOMM_LOG("[TX] Retry %d/%d for seq=%d to 0x%02X\n",
                                      pending.retryCount, maxRetries,
                                      pending.sequence, pending.destAddr);

                        // Put back in queue
                        xQueueSend(ackPendingQueue, &pending, 0);
                    }
                    else
                    {
                        failedTxCount++;
                        LORASCOMM_LOG("[TX] Retry failed for seq=%d\n", pending.sequence);
                    }
                }
                else
                {
                    // Max retries reached, give up
                    failedTxCount++;
                    LORASCOMM_LOG("[TX] Max retries reached for seq=%d to 0x%02X\n",
                                  pending.sequence, pending.destAddr);
                    // Don't put back in queue (dropped)
                }
            }
            else
            {
                // Not timed out yet, put back in queue
                xQueueSend(ackPendingQueue, &pending, 0);
            }
        }
    }
}