// Add to NetworkManager.h

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <vector>
#include "EventBus.h"
#include "MessageParser.h"
#include "../lib/LoRaSComm/LoRaSComm.h"
#include "../Protocol.h"
#include "../Config.h"
#include "MessageHandler.h"
#include "../lib/Logging/LogManager.h"

// Node discovery struct
struct NodeInfo {
    uint8_t address;
    int16_t rssi;
    float snr;
    unsigned long lastSeen;
    bool isAlive;
};

// Poll response callback
typedef std::function<void(uint8_t addr, const String& data)> PollResponseCallback;

enum NetworkRole {
    ROLE_UNDEFINED,
    ROLE_SERVER,
    ROLE_CLIENT
};

class NetworkManager {
public:
    NetworkManager(EventBus& eventBus, MessageHandler& confHandler);
    
    void begin();
    void setAsServer();
    void setAsClient(uint8_t address);
    
    // === Existing send methods ===
    void sendToMain(const String& message);
    void broadcast(const String& message);
    void sendTo(uint8_t address, const String& message);
    void sendToAndWait(uint8_t address, const String& message,EventGroupHandle_t event,EventBits_t bits);
    bool sendToAll(const std::vector<uint8_t>& addresses, const String& message);
    void sendToUnreliable(uint8_t address, const String& message);

    // === Why i even made all of this ? ===
    
    // Method 1: Sequential poll (simple, reliable)
    // Polls each node one by one, returns list of alive nodes
    std::vector<NodeInfo> pollNodesSequential(const std::vector<uint8_t>& addresses, uint32_t timeoutMs = 2000);
    
    // Method 2: Broadcast poll with TDMA slots
    // Broadcasts poll request, nodes respond in assigned time slots
    std::vector<NodeInfo> pollNodesTDMA(const std::vector<uint8_t>& addresses, uint16_t slotMs = 300);
    
    // Method 3: Discovery poll (finds unknown nodes)
    // Broadcasts poll, any node can respond (with random backoff)
    std::vector<NodeInfo> discoverNodes(uint32_t listenDurationMs = 3000);
    
    // Node list management
    void addKnownNode(uint8_t address);
    void removeKnownNode(uint8_t address);
    std::vector<uint8_t> getKnownNodes() const;
    NodeInfo* getNodeInfo(uint8_t address);
    
    // Handlers
    static void handleReceived(const ReceivedPacket& packet);  
    void networkDiscoverCallback(Event e);
    void broadcastReset();

private:
    void onPacketReceived(const ReceivedPacket& packet);
    void handlePollRequest(const ReceivedPacket& packet);
    void updateNodeInfo(uint8_t address, int16_t rssi, float snr);
    
    SX1278 radio = SX1278(new Module(LORA_NSS, LORA_DIO0, LORA_RST, LORA_DIO1));
    LoRaSComm SComm = LoRaSComm(&radio);
    EventBus& eventBus;
    MessageHandler& confHandler;
    
    static NetworkManager* s_instance;
    NetworkRole role;
    uint8_t myAddress;
    uint8_t masterAddress;
    bool networkReady;
    
    // Known nodes tracking
    std::vector<NodeInfo> knownNodes;
    SemaphoreHandle_t nodeListMutex;
};

#endif // NETWORK_MANAGER_H