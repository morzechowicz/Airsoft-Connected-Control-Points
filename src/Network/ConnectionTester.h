#ifndef CONNECTION_TESTER_H
#define CONNECTION_TESTER_H

#include <Arduino.h>
#include "NetworkManager.h"
#include "Hardware/HardwareManager.h"
#include "../lib/Logging/LogManager.h"
#include "EventBus.h"
#include "Protocol.h"

struct TestResponse {
    uint8_t packetId = 0;
    uint8_t retryCount = 0;
    int16_t rssi = 0;
    float snr = 0.0f;
};

struct NodeEntry {
    uint8_t address = 0;
    bool found = false;
    int16_t rssi = 0;
    float snr = 0.0f;
    uint8_t generatedPackets = 0;
    uint8_t lastresponse = 0;
    TestResponse responses[10];
};

class ConnectionTester
{
public:
    ConnectionTester(NetworkManager* networkManager, HardwareManager* hardwareManager, EventBus* eventBus);
    
    void runTest(uint8_t nodeCount);

    void addNewNode(uint8_t address, int16_t rssi, float snr);
private:
    NetworkManager *networkManager;
    HardwareManager *hardwareManager;
    EventBus *eventBus;

    uint8_t nodeCount;
    uint8_t nodesFound;

    uint8_t currentNodeIndex;
    NodeEntry Nodes[10];

    void broadcastTestSearch();
    void sendTestToNode(int nodeId);
    void handleBroadcastResponse(Event e);
    void handleBroadcast(Event e);
    void handleDirectResponse(Event e);
    void handleDirect(Event e);

    void createTestTask();
    void testTask(void *pvParameters);

    //helpers
    int findNodeIndexByAddress(uint8_t address);
};

#endif // CONNECTION_TESTER_H