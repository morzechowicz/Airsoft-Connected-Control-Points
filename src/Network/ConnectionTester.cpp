#include "ConnectionTester.h"

ConnectionTester::ConnectionTester(NetworkManager *networkManager, HardwareManager *hardwareManager, EventBus *eventBus) :
    networkManager(networkManager),
    hardwareManager(hardwareManager),
    eventBus(eventBus)
{
    eventBus->subscribe(TEST_BEGIN, [this](Event e) { testConnection(e); });
    eventBus->subscribe(TEST_DIRECT, [this](Event e) { testConnectionCommand(e); });
    eventBus->subscribe(TEST_DR_RESPONSE, [this](Event e) { testConnectionResponse(e); });

}

void ConnectionTester::testConnection(Event e)
{
    uint8_t targetNode = e.data1;
    uint16_t packetId = e.data2;
    LOG_INFO("CONNECTION_TESTER", "Received test connection command with node %d", targetNode);
    if(networkManager)
    {
        String msg = Protocol::buildTestDrMsg(LORA_ADDRESS, packetId); //i dont think i need second parameter
        networkManager->sendTo(targetNode, msg);
        LOG_DEBUG("CONNECTION_TESTER", "Sent test connection message to node %d", targetNode);
    }
}

void ConnectionTester::testConnectionCommand(Event e)
{
    uint8_t sourceId = e.data1;
    int16_t rssi = e.data2;
    float snr = e.data3;
    uint8_t packetId = e.data4;
    LOG_INFO("CONNECTION_TESTER", "Received test connection command from node %d with RSSI %d and SNR %.2f", sourceId, rssi, snr);
    if(networkManager)
    {
        String msg = Protocol::buildTestDrResponseMsg(LORA_ADDRESS, packetId, 1, rssi, snr);
        networkManager->sendTo(sourceId, msg);
        LOG_DEBUG("CONNECTION_TESTER", "Sent test connection response to node %d", sourceId);
    }
}

void ConnectionTester::testConnectionResponse(Event e)
{
    LOG_INFO("CONNECTION_TESTER", "Received test connection response from node %d with RSSI %d and SNR %.2f", e.data1, e.data3, e.data4);

}
