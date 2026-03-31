#include "NetworkManager.h"

NetworkManager *NetworkManager::s_instance = nullptr;

// Update constructor
NetworkManager::NetworkManager(EventBus &eventBus, ConfigurationHandler &confHandler) 
    : eventBus(eventBus), confHandler(confHandler)
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
    eventBus.subscribe(NETWORK_DISCOVER, [this](Event e) { 
        this->networkDiscoverCallback(e); 
    });
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

        SComm.sendReliable(address, (uint8_t *)msg, strlen(msg),nullptr,0);
    }
}

bool NetworkManager::sendToAll(const std::vector<uint8_t>& addresses, const String& message)
{
    if (!networkReady) return false;

    EventGroupHandle_t ackEvents = xEventGroupCreate();
    EventBits_t expectedBits = 0;

    char msg[120];
    snprintf(msg, sizeof(msg), "%s", message.c_str());

    // Fire all sends without waiting
    for (int i = 0; i < addresses.size(); i++) {
        if (addresses[i] == myAddress) continue;

        EventBits_t bit = (1 << i);
        expectedBits |= bit;

        SComm.sendReliable(
            addresses[i],
            (uint8_t*)msg, strlen(msg),
            ackEvents, bit           // <-- new params
        );
    }

    // Block until all ACKed or timeout
    EventBits_t result = xEventGroupWaitBits(
        ackEvents,
        expectedBits,
        pdTRUE,              // clear bits on exit
        pdTRUE,              // wait for ALL bits
        pdMS_TO_TICKS(5000)
    );

    vEventGroupDelete(ackEvents);

    if ((result & expectedBits) == expectedBits) {
        LOG_INFO("NETWORK", "All nodes confirmed");
        return true;
    }

    // Log exactly who failed
    for (int i = 0; i < addresses.size(); i++) {
        if (!(result & (1 << i))) {
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

void NetworkManager::broadcastReset() {
    String msg = Protocol::buildPowerResetMsg();
    broadcast(msg);
}

void NetworkManager::onPacketReceived(const ReceivedPacket &packet)
{
    Message msg;
    msg = Protocol::parse((const char *)packet.data, packet.dataLen);
    LOG_INFO("NETWORK", "Received message: %s", (const char *)packet.data);
    bool result = confHandler.handleCommand(msg);
    if (!result)
    {
        LOG_ERROR("NETWORK", "Something went wrong at Configuration handler");
    }
}

// ========== METHOD 1: Sequential Poll ==========
std::vector<NodeInfo> NetworkManager::pollNodesSequential(
    const std::vector<uint8_t>& addresses, 
    uint32_t timeoutMs) 
{
    std::vector<NodeInfo> results;
    
    if (!networkReady) {
        Serial.println("[NETWORK] Not ready for polling");
        return results;
    }
    
    Serial.printf("[NETWORK] Sequential poll of %d nodes\n", addresses.size());
    
    // Temporarily reduce ACK timeout for faster polling
    uint32_t originalTimeout = 2000; // Store original
    SComm.setAckTimeout(timeoutMs);
    
    for (uint8_t addr : addresses) {
        if (addr == myAddress) continue; // Skip ourselves
        
        Serial.printf("[NETWORK] Polling node 0x%02X... ", addr);
        
        String pingMsg = "PING";
        char msg[120];
        snprintf(msg, sizeof(msg), "%s", pingMsg.c_str());
        
        // Use reliable send - ACK means node is alive
        EventGroupHandle_t ackEvent = xEventGroupCreate();
        EventBits_t ackBit = 0x01;
        
        SComm.sendReliable(addr, (uint8_t*)msg, strlen(msg), ackEvent, ackBit);
        
        // Wait for ACK
        EventBits_t result = xEventGroupWaitBits(
            ackEvent,
            ackBit,
            pdTRUE,
            pdTRUE,
            pdMS_TO_TICKS(timeoutMs)
        );
        
        vEventGroupDelete(ackEvent);
        
        NodeInfo info;
        info.address = addr;
        info.rssi = SComm.getLastRSSI();
        info.snr = SComm.getLastSNR();
        info.lastSeen = millis();
        info.isAlive = (result & ackBit) != 0;
        
        if (info.isAlive) {
            Serial.printf("ALIVE (RSSI: %d)\n", info.rssi);
            updateNodeInfo(addr, info.rssi, info.snr);
        } else {
            Serial.println("TIMEOUT");
        }
        
        results.push_back(info);
        
        // Small delay between polls
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // Restore original timeout
    SComm.setAckTimeout(originalTimeout);
    
    Serial.printf("[NETWORK] Poll complete: %d/%d alive\n", 
                  std::count_if(results.begin(), results.end(), 
                               [](const NodeInfo& n) { return n.isAlive; }),
                  results.size());
    
    return results;
}

// ========== METHOD 2: TDMA Poll ==========
std::vector<NodeInfo> NetworkManager::pollNodesTDMA(
    const std::vector<uint8_t>& addresses, 
    uint16_t slotMs) 
{
    std::vector<NodeInfo> results;
    
    if (!networkReady) return results;
    
    Serial.printf("[NETWORK] TDMA poll of %d nodes (slot: %dms)\n", 
                  addresses.size(), slotMs);
    
    // Build poll request message
    // Format: "POLL_TDMA;slotMs;addr1,addr2,addr3"
    String pollMsg = "POLL_TDMA;" + String(slotMs) + ";";
    for (int i = 0; i < addresses.size(); i++) {
        if (addresses[i] == myAddress) continue;
        pollMsg += String(addresses[i], HEX);
        if (i < addresses.size() - 1) pollMsg += ",";
    }
    
    // Broadcast poll request
    broadcast(pollMsg);
    
    // Listen for responses in time slots
    uint32_t startTime = millis();
    uint32_t totalDuration = addresses.size() * slotMs + 500; // Extra 500ms buffer
    
    while (millis() - startTime < totalDuration) {
        if (SComm.available()) {
            ReceivedPacket packet;
            if (SComm.receive(packet)) {
                // Check if this is a poll response
                String data((char*)packet.data, packet.dataLen);
                if (data.startsWith("POLL_RESP")) {
                    NodeInfo info;
                    info.address = packet.srcAddr;
                    info.rssi = packet.rssi;
                    info.snr = packet.snr;
                    info.lastSeen = millis();
                    info.isAlive = true;
                    
                    results.push_back(info);
                    updateNodeInfo(info.address, info.rssi, info.snr);
                    
                    Serial.printf("[NETWORK] Node 0x%02X responded (RSSI: %d)\n", 
                                  info.address, info.rssi);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    Serial.printf("[NETWORK] TDMA poll complete: %d/%d responded\n", 
                  results.size(), addresses.size());
    
    return results;
}

// ========== METHOD 3: Discovery Poll ==========
std::vector<NodeInfo> NetworkManager::discoverNodes(uint32_t listenDurationMs) 
{
    std::vector<NodeInfo> results;
    
    if (!networkReady) return results;
    
    Serial.printf("[NETWORK] Discovery poll (listening for %dms)\n", listenDurationMs);
    
    // Broadcast discovery request
    String discoveryMsg = "DISCOVER";
    broadcast(discoveryMsg);
    
    // Listen for responses
    uint32_t startTime = millis();
    
    while (millis() - startTime < listenDurationMs) {
        if (SComm.available()) {
            ReceivedPacket packet;
            if (SComm.receive(packet)) {
                String data((char*)packet.data, packet.dataLen);
                
                // Check if this is a discovery response
                if (data.startsWith("DISC_RESP")) {
                    // Check if we already got this node
                    bool alreadyFound = false;
                    for (const auto& node : results) {
                        if (node.address == packet.srcAddr) {
                            alreadyFound = true;
                            break;
                        }
                    }
                    
                    if (!alreadyFound) {
                        NodeInfo info;
                        info.address = packet.srcAddr;
                        info.rssi = packet.rssi;
                        info.snr = packet.snr;
                        info.lastSeen = millis();
                        info.isAlive = true;
                        
                        results.push_back(info);
                        updateNodeInfo(info.address, info.rssi, info.snr);
                        
                        Serial.printf("[NETWORK] Discovered node 0x%02X (RSSI: %d)\n", 
                                      info.address, info.rssi);
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    Serial.printf("[NETWORK] Discovery complete: found %d nodes\n", results.size());
    
    return results;
}

// ========== Node Management ==========
void NetworkManager::addKnownNode(uint8_t address) {
    if (xSemaphoreTake(nodeListMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Check if already exists
        for (auto& node : knownNodes) {
            if (node.address == address) {
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

void NetworkManager::removeKnownNode(uint8_t address) {
    if (xSemaphoreTake(nodeListMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        knownNodes.erase(
            std::remove_if(knownNodes.begin(), knownNodes.end(),
                          [address](const NodeInfo& n) { return n.address == address; }),
            knownNodes.end()
        );
        xSemaphoreGive(nodeListMutex);
    }
}

std::vector<uint8_t> NetworkManager::getKnownNodes() const {
    std::vector<uint8_t> addresses;
    if (xSemaphoreTake(nodeListMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        for (const auto& node : knownNodes) {
            addresses.push_back(node.address);
        }
        xSemaphoreGive(nodeListMutex);
    }
    return addresses;
}

NodeInfo* NetworkManager::getNodeInfo(uint8_t address) {
    if (xSemaphoreTake(nodeListMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        for (auto& node : knownNodes) {
            if (node.address == address) {
                xSemaphoreGive(nodeListMutex);
                return &node;
            }
        }
        xSemaphoreGive(nodeListMutex);
    }
    return nullptr;
}

void NetworkManager::updateNodeInfo(uint8_t address, int16_t rssi, float snr) {
    if (xSemaphoreTake(nodeListMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        bool found = false;
        for (auto& node : knownNodes) {
            if (node.address == address) {
                node.rssi = rssi;
                node.snr = snr;
                node.lastSeen = millis();
                node.isAlive = true;
                found = true;
                break;
            }
        }
        
        if (!found) {
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



void NetworkManager::handlePollRequest(const ReceivedPacket& packet) {
    String data((char*)packet.data, packet.dataLen);
    
    // Parse: "POLL_TDMA;300;1,2,5,A"
    int firstSemi = data.indexOf(';');
    int secondSemi = data.indexOf(';', firstSemi + 1);
    
    if (firstSemi == -1 || secondSemi == -1) return;
    
    uint16_t slotMs = data.substring(firstSemi + 1, secondSemi).toInt();
    String addrList = data.substring(secondSemi + 1);
    
    // Find my position in the list
    int pos = 0;
    int idx = 0;
    while (idx < addrList.length()) {
        int comma = addrList.indexOf(',', idx);
        if (comma == -1) comma = addrList.length();
        
        String addrStr = addrList.substring(idx, comma);
        uint8_t addr = strtol(addrStr.c_str(), NULL, 16);
        
        if (addr == myAddress) {
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