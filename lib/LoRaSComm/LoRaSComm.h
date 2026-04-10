/*
 * LoRaSComm - ESP32 LoRa Communication Library
 * Transport layer for SX1278 with reliable/unreliable transmission
 */

#ifndef LORASCOMM_H
#define LORASCOMM_H

#include <Arduino.h>
#include <RadioLib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "LoRaSCommPacket.h"
#include "../Logging/LogManager.h"

// FreeRTOS Configuration
#define LORASCOMM_RX_QUEUE_SIZE 10
#define LORASCOMM_TX_QUEUE_SIZE 10
#define LORASCOMM_ACK_PENDING_SIZE 5

#define LORASCOMM_RX_TASK_STACK 4096
#define LORASCOMM_TX_TASK_STACK 4096
#define LORASCOMM_USER_TASK_STACK 8192

#define LORASCOMM_RX_TASK_PRIORITY 3
#define LORASCOMM_TX_TASK_PRIORITY 2
#define LORASCOMM_USER_TASK_PRIORITY 1

// Protocol defaults
#define LORASCOMM_DEFAULT_MAX_RETRIES 5
#define LORASCOMM_DEFAULT_ACK_TIMEOUT 2000  // ms

// Listen Before Talk (LBT)
#define LORASCOMM_LBT_MAX_BACKOFF_MS   500  // max random backoff window
#define LORASCOMM_LBT_MIN_BACKOFF_MS    50
#define LORASCOMM_LBT_MAX_ATTEMPTS     8    // give up after this many attempts

// Repeater
#define LORASCOMM_REPEATER_SEEN_TABLE_SIZE  16
#define LORASCOMM_REPEATER_SEEN_EXPIRY_MS   10000  // forget seen packets after 10s

// Received packet info
struct ReceivedPacket {
    uint8_t srcAddr;
    uint8_t data[LORASCOMM_MAX_PAYLOAD_SIZE];
    size_t dataLen;
    int16_t rssi;
    float snr;
    
    ReceivedPacket() : srcAddr(0), dataLen(0), rssi(0), snr(0.0f) {
        memset(data, 0, LORASCOMM_MAX_PAYLOAD_SIZE);
    }
};

// Pending ACK tracking
struct PendingAck {
    uint8_t destAddr;
    uint8_t sequence;
    uint8_t retryCount;
    uint32_t sentTime;
    uint8_t data[LORASCOMM_MAX_PAYLOAD_SIZE];
    size_t dataLen;
    bool active;

    EventGroupHandle_t eventGroup;  // nullptr for fire-and-forget
    EventBits_t eventBit;           // bit to set on ACK
    PendingAck() : destAddr(0), sequence(0), retryCount(0), 
                   sentTime(0), dataLen(0), active(false) {
        memset(data, 0, LORASCOMM_MAX_PAYLOAD_SIZE);
    }
};

// TX Queue item
struct TxQueueItem {
    uint8_t destAddr;
    uint8_t data[LORASCOMM_MAX_PAYLOAD_SIZE];
    size_t dataLen;
    LoRaSCommPacketType packetType;
    EventGroupHandle_t eventGroup;  // nullptr for fire-and-forget
    EventBits_t eventBit;           // bit to set on ACK

    // For ACK/NACK: sequence number is stored in data[0] (no real payload)
    // For forwarded packets: preserves original srcAddr and sequence
    uint8_t originalSrcAddr;        // used when forwarding
    uint8_t originalSequence;       // used when forwarding
    bool isForward;                 // true = repeater forward, bypass own sequence counter

    TxQueueItem() : destAddr(0), dataLen(0), packetType(PACKET_DATA_UNRELIABLE),
                    eventGroup(nullptr), eventBit(0),
                    originalSrcAddr(0), originalSequence(0), isForward(false) {
        memset(data, 0, LORASCOMM_MAX_PAYLOAD_SIZE);
    }
};

// Repeater deduplication table entry
struct RepeaterSeenEntry {
    uint8_t srcAddr;
    uint8_t sequence;
    uint32_t timestamp;

    RepeaterSeenEntry() : srcAddr(0), sequence(0), timestamp(0) {}
};

class LoRaSComm {
public:
    LoRaSComm(SX1278* radioModule);
    ~LoRaSComm();
    
    // Initialization - starts FreeRTOS tasks
    bool begin(uint8_t myAddress, 
               float frequency = 915.0,
               int8_t power = 17,
               uint8_t spreadingFactor = 7,
               float signalBandwidth = 125.0,
               uint8_t codingRate = 5);
    
    // Stop all tasks and cleanup
    void end();
    
    // Send methods (non-blocking, add to queue)
    bool sendUnreliable(uint8_t dest, const uint8_t* data, size_t len);
    bool sendReliable(uint8_t dest, const uint8_t* data, size_t len, EventGroupHandle_t eventGroup, EventBits_t eventBit);
    bool testSendAck(uint8_t dest);
    
    // Receive (non-blocking, reads from queue)
    bool available();
    bool receive(ReceivedPacket& packet);
    
    // Configuration
    void setMaxRetries(uint8_t retries);
    void setAckTimeout(uint32_t ms);
    void setAddress(uint8_t addr);
    
    // Callbacks (called from user task)
    void onReceive(void (*callback)(const ReceivedPacket& packet));
    
    // Repeater mode - when enabled, forwards packets not addressed to this node
    void setRepeaterMode(bool enabled) { repeaterMode = enabled; }
    bool isRepeaterMode() const { return repeaterMode; }
    
    // Diagnostics
    uint32_t getTxPackets() const { return txPacketCount; }
    uint32_t getRxPackets() const { return rxPacketCount; }
    uint32_t getFailedTx() const { return failedTxCount; }
    int16_t getLastRSSI() const { return lastRssi; }
    float getLastSNR() const { return lastSnr; }
    
private:
    // FreeRTOS tasks (static for task creation)
    static void rxTask(void* params);
    static void txTask(void* params);
    static void userTask(void* params);
    
    // Internal methods
    void enqueueAck(uint8_t dest, uint8_t sequence);
    void enqueueNack(uint8_t dest, uint8_t sequence);
    void sendAck(uint8_t dest, uint8_t sequence);   // kept for testSendAck only
    void sendNack(uint8_t dest, uint8_t sequence);  // kept for symmetry
    bool transmitPacket(const uint8_t* buffer, size_t len);
    void handleReceivedPacket(const LoRaSCommPacket& packet);
    void processAckTimeout();

    // Repeater internals
    void forwardPacket(const LoRaSCommPacket& packet);
    bool repeaterAlreadySeen(uint8_t src, uint8_t seq);
    void repeaterMarkSeen(uint8_t src, uint8_t seq);
    
    // Radio instance
    SX1278* radio;
    
    // State
    uint8_t myAddress;
    uint8_t currentSequence;
    uint8_t maxRetries;
    uint32_t ackTimeout;
    bool initialized;
    
    // FreeRTOS objects
    QueueHandle_t rxQueue;
    QueueHandle_t txQueue;
    QueueHandle_t ackPendingQueue;
    SemaphoreHandle_t radioMutex;
    TaskHandle_t rxTaskHandle;
    TaskHandle_t txTaskHandle;
    TaskHandle_t userTaskHandle;
    
    // Stats
    uint32_t txPacketCount;
    uint32_t rxPacketCount;
    uint32_t failedTxCount;
    int16_t lastRssi;
    float lastSnr;
    
    // Repeater state
    bool repeaterMode;
    RepeaterSeenEntry seenTable[LORASCOMM_REPEATER_SEEN_TABLE_SIZE];
    uint8_t seenHead;
    uint32_t repeaterForwardCount;
    
    // Callback
    void (*receiveCallback)(const ReceivedPacket& packet);
};

#endif // LORASCOMM_H