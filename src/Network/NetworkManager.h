#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include "EventBus.h"
#include "MessageParser.h"
#include "../lib/LoRaSComm/LoRaSComm.h"
#include "../Protocol.h"
#include "../Config.h"
#include "ConfigurationHandler.h"

// Other nodes discovery struct
struct NodeInfo {
    uint8_t address;
    int16_t rssi;
    float snr;
    unsigned long lastSeen;
};

enum NetworkRole {
    ROLE_UNDEFINED, //who knows
    ROLE_SERVER, //which means it have both client and server
    ROLE_CLIENT //which means it have only client
};

class NetworkManager {
public:
    NetworkManager(EventBus& eventBus,ConfigurationHandler& confHandler);
    
    void begin();
    void setAsServer();
    void setAsClient(uint8_t addres);
    
    // Send methods (game components use these)
    void sendToMaster(const String& message);
    void broadcast(const String& message);
    void sendTo(uint8_t address, const String& message);
    bool sendToAll(const std::vector<uint8_t>& addresses, const String& message);
   
    static void handleReceived(const ReceivedPacket& packet);  
    void networkDiscoverCallback(Event e);
    void broadcastReset();
private:
    void onPacketReceived(const ReceivedPacket& packet);

    SX1278 radio = SX1278(new Module(LORA_NSS, LORA_DIO0, LORA_RST, LORA_DIO1));
    LoRaSComm SComm = LoRaSComm(&radio);
    EventBus& eventBus;
    ConfigurationHandler& confHandler;
    
    static NetworkManager* s_instance;
    NetworkRole role;
    uint8_t myAddress;
    uint8_t masterAddress;
    bool networkReady;
};

#endif // NETWORK_MANAGER_H