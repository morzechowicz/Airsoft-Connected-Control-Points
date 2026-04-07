#include "NetworkManager.h"

NetworkManager *NetworkManager::s_instance = nullptr;

// Update constructor
NetworkManager::NetworkManager(EventBus &eventBus, MessageHandler &msgHandler)
    : eventBus(eventBus), msgHandler(msgHandler)
{
    role = ROLE_UNDEFINED;
    s_instance = this;
    nodeListMutex = xSemaphoreCreateMutex();
}

void NetworkManager::begin()
{
    myAddress = LORA_ADDRESS;
    networkReady = true;
    SComm.begin(LORA_ADDRESS, LORA_FREQUENCY, LORA_TX_POWER,
                LORA_SPREADING_FACTOR, LORA_SIGNAL_BANDWIDTH, LORA_CODING_RATE);
    SComm.onReceive(NetworkManager::handleReceived);
    eventBus.subscribe(NETWORK_DISCOVER, [this](Event e)
                       { this->networkDiscoverCallback(e); });
    eventBus.subscribe(FORWARD, [this](Event e) 
    { this->forwardMsgCallback(e);});
}

void NetworkManager::setAsServer()
{
    this->role = ROLE_SERVER;
}
// address of a master node
void NetworkManager::setAsClient(uint8_t addres)
{
    this->masterAddress = addres;
    this->role = ROLE_CLIENT;
}

void NetworkManager::sendToMain(const String &message)
{
    LOG_INFO("NETWORK", "Sending message to master (0x%02X)", masterAddress);

    if (networkReady && role == ROLE_CLIENT)
    {
        sendTo(masterAddress, message);
    }
}

void NetworkManager::broadcast(const String &message)
{
    if (networkReady)
    {
        char msg[120];
        snprintf(msg, sizeof(msg), "%s", message.c_str());

        // Broadcast address is 0xFF
        SComm.sendUnreliable(0xFF, (uint8_t *)msg, strlen(msg));
    }
}

void NetworkManager::sendTo(uint8_t address, const String &message)
{
    if (networkReady)
    {
        char msg[120];
        snprintf(msg, sizeof(msg), "%s", message.c_str());

        SComm.sendReliable(address, (uint8_t *)msg, strlen(msg), nullptr, 0);
    }
}

void NetworkManager::sendToAndWait(uint8_t address, const String &message, EventGroupHandle_t event, EventBits_t bits)
{
    if (networkReady)
    {
        char msg[120];
        snprintf(msg, sizeof(msg), "%s", message.c_str());

        SComm.sendReliable(address, (uint8_t *)msg, strlen(msg), event, bits);
    }
}

bool NetworkManager::sendToAll(const std::vector<uint8_t> &addresses, const String &message)
{
    if (!networkReady)
        return false;

    EventGroupHandle_t ackEvents = xEventGroupCreate();
    EventBits_t expectedBits = 0;

    char msg[120];
    snprintf(msg, sizeof(msg), "%s", message.c_str());

    // Fire all sends without waiting
    for (int i = 0; i < addresses.size(); i++)
    {
        if (addresses[i] == myAddress)
            continue;

        EventBits_t bit = (1 << i);
        expectedBits |= bit;

        SComm.sendReliable(
            addresses[i],
            (uint8_t *)msg, strlen(msg),
            ackEvents, bit // <-- new params
        );
    }

    // Block until all ACKed or timeout
    EventBits_t result = xEventGroupWaitBits(
        ackEvents,
        expectedBits,
        pdTRUE, // clear bits on exit
        pdTRUE, // wait for ALL bits
        pdMS_TO_TICKS(5000));

    vEventGroupDelete(ackEvents);

    if ((result & expectedBits) == expectedBits)
    {
        LOG_INFO("NETWORK", "All nodes confirmed");
        return true;
    }

    // Log exactly who failed
    for (int i = 0; i < addresses.size(); i++)
    {
        if (!(result & (1 << i)))
        {
            LOG_ERROR("NETWORK", "Node 0x%02X did not ACK", addresses[i]);
        }
    }
    return false;
}

void NetworkManager::sendToUnreliable(uint8_t address, const String &message)
{
    if (networkReady)
    {
        char msg[120];
        snprintf(msg, sizeof(msg), "%s", message.c_str());

        SComm.sendUnreliable(address, (uint8_t *)msg, strlen(msg));
    }
}

void NetworkManager::handleReceived(const ReceivedPacket &packet)
{
    if (s_instance)
    {
        s_instance->onPacketReceived(packet); // Forward to instance method
    }
}

void NetworkManager::networkDiscoverCallback(Event e)
{
    if (s_instance)
    {
        s_instance->setAsClient(e.data1);
        String response = Protocol::buildDiscoverResponse(LORA_ADDRESS);
        LOG_INFO("NETWORK", "Responding with: %s", response.c_str());
        s_instance->sendToMain(response);
    }
}

void NetworkManager::forwardMsgCallback(Event e)
{
    int forwardTo = e.data1;
    if(forwardTo == 0)
    {
        LOG_ERROR("NETWORK","Forwarding was called with id 0");
        return;
    }
    sendTo(e.data1,e.message);
}

void NetworkManager::broadcastReset()
{
    String msg = Protocol::buildPowerResetMsg();
    broadcast(msg);
}

void NetworkManager::onPacketReceived(const ReceivedPacket &packet)
{
    Message msg;
    msg = Protocol::parse((const char *)packet.data, packet.dataLen);
    LOG_INFO("NETWORK", "Received message: %s", (const char *)packet.data);
    bool result = msgHandler.handleCommand(msg, (const char *)packet.data);
    if (!result)
    {
        LOG_ERROR("NETWORK", "Something went wrong at Configuration handler");
    }
}

// ========== Node Management ==========
void NetworkManager::addKnownNode(uint8_t address)
{
    if (xSemaphoreTake(nodeListMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        // Check if already exists
        for (auto &node : knownNodes)
        {
            if (node.address == address)
            {
                xSemaphoreGive(nodeListMutex);
                return;
            }
        }

        // Add new node
        NodeInfo info;
        info.address = address;
        info.rssi = 0;
        info.snr = 0.0f;
        info.lastSeen = millis();
        info.isAlive = false;

        knownNodes.push_back(info);
        Serial.printf("[NETWORK] Added node 0x%02X to known list\n", address);

        xSemaphoreGive(nodeListMutex);
    }
}

void NetworkManager::removeKnownNode(uint8_t address)
{
    if (xSemaphoreTake(nodeListMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        knownNodes.erase(
            std::remove_if(knownNodes.begin(), knownNodes.end(),
                           [address](const NodeInfo &n)
                           { return n.address == address; }),
            knownNodes.end());
        xSemaphoreGive(nodeListMutex);
    }
}

std::vector<uint8_t> NetworkManager::getKnownNodes() const
{
    std::vector<uint8_t> addresses;
    if (xSemaphoreTake(nodeListMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        for (const auto &node : knownNodes)
        {
            addresses.push_back(node.address);
        }
        xSemaphoreGive(nodeListMutex);
    }
    return addresses;
}

NodeInfo *NetworkManager::getNodeInfo(uint8_t address)
{
    if (xSemaphoreTake(nodeListMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        for (auto &node : knownNodes)
        {
            if (node.address == address)
            {
                xSemaphoreGive(nodeListMutex);
                return &node;
            }
        }
        xSemaphoreGive(nodeListMutex);
    }
    return nullptr;
}

void NetworkManager::updateNodeInfo(uint8_t address, int16_t rssi, float snr)
{
    if (xSemaphoreTake(nodeListMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        bool found = false;
        for (auto &node : knownNodes)
        {
            if (node.address == address)
            {
                node.rssi = rssi;
                node.snr = snr;
                node.lastSeen = millis();
                node.isAlive = true;
                found = true;
                break;
            }
        }

        if (!found)
        {
            // Auto-add new node
            NodeInfo info;
            info.address = address;
            info.rssi = rssi;
            info.snr = snr;
            info.lastSeen = millis();
            info.isAlive = true;
            knownNodes.push_back(info);
        }

        xSemaphoreGive(nodeListMutex);
    }
}

void NetworkManager::handlePollRequest(const ReceivedPacket &packet)
{
    String data((char *)packet.data, packet.dataLen);

    // Parse: "POLL_TDMA;300;1,2,5,A"
    int firstSemi = data.indexOf(';');
    int secondSemi = data.indexOf(';', firstSemi + 1);

    if (firstSemi == -1 || secondSemi == -1)
        return;

    uint16_t slotMs = data.substring(firstSemi + 1, secondSemi).toInt();
    String addrList = data.substring(secondSemi + 1);

    // Find my position in the list
    int pos = 0;
    int idx = 0;
    while (idx < addrList.length())
    {
        int comma = addrList.indexOf(',', idx);
        if (comma == -1)
            comma = addrList.length();

        String addrStr = addrList.substring(idx, comma);
        uint8_t addr = strtol(addrStr.c_str(), NULL, 16);

        if (addr == myAddress)
        {
            // Found my slot! Wait and respond
            uint32_t myDelay = pos * slotMs;
            Serial.printf("[NETWORK] TDMA poll: responding in slot %d (%dms)\n", pos, myDelay);

            vTaskDelay(pdMS_TO_TICKS(myDelay));

            String response = "POLL_RESP;" + String(myAddress, HEX);
            sendToUnreliable(packet.srcAddr, response);
            return;
        }

        pos++;
        idx = comma + 1;
    }
}