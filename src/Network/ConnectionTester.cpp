#include "ConnectionTester.h"

ConnectionTester::ConnectionTester(NetworkManager *networkManager, HardwareManager *hardwareManager, EventBus *eventBus)
    : networkManager(networkManager), hardwareManager(hardwareManager), eventBus(eventBus)
{
    // add eventbuss scubscribers
    eventBus->subscribe(EventType::TEST_BROADCAST, [this](Event e)
                        { this->handleBroadcast(e); });
    eventBus->subscribe(EventType::TEST_BR_RESPONSE, [this](Event e)
                        { this->handleBroadcastResponse(e); });
    eventBus->subscribe(EventType::TEST_DIRECT, [this](Event e)
                        { this->handleDirect(e); });
    eventBus->subscribe(EventType::TEST_DR_RESPONSE, [this](Event e)
                        { this->handleDirectResponse(e); });
}

void ConnectionTester::runTest(uint8_t nodeCount)
{
    this->nodeCount = nodeCount;
    this->nodesFound = 0;

    for (int i = 0; i < nodeCount; i++)
    {
        Nodes[i].found = false;
        Nodes[i].address = 0;
        Nodes[i].rssi = 0;
        Nodes[i].snr = 0;
        for (int j = 0; j < 10; j++)
        {
            Nodes[i].responses[j].packetId = 0;
            Nodes[i].responses[j].retryCount = 0;
            Nodes[i].responses[j].rssi = 0;
            Nodes[i].responses[j].snr = 0;
        }
    }
    createTestTask();
}

void ConnectionTester::addNewNode(uint8_t address, int16_t rssi, float snr)
{
    //check if node is already in list
    for (int i = 0; i < nodesFound; i++)
    {
        if (Nodes[i].address == address)        {
            return;
        }
    }
    if (nodesFound < nodeCount)
    {
        Nodes[nodesFound].address = address;
        Nodes[nodesFound].rssi = rssi;
        Nodes[nodesFound].snr = snr;
        Nodes[nodesFound].found = true;
        nodesFound++;
    }
}

void ConnectionTester::broadcastTestSearch()
{
    LOG_INFO("ConnectionTester", "Broadcasting test search...");
    String msg = Protocol::buildTestBrMsg(LORA_ADDRESS);
    networkManager->broadcast(msg);
}

void ConnectionTester::sendTestToNode(int nodeId)
{
    uint8_t packetid = Nodes[nodeId].generatedPackets++;

    LOG_INFO("ConnectionTester", "Sending test message nr %d to node %d at address %d", packetid, nodeId, Nodes[nodeId].address);
    String msg = Protocol::buildTestDrMsg(LORA_ADDRESS, packetid);
    EventGroupHandle_t responseEvent = xEventGroupCreate();
    EventBits_t responseBit = 0x01;
    networkManager->sendToAndWait(Nodes[nodeId].address, msg, responseEvent, responseBit);
    EventBits_t result = xEventGroupWaitBits(responseEvent, responseBit, pdTRUE, pdFALSE, pdMS_TO_TICKS(5000));
    if (result & responseBit)
    {
        LOG_INFO("ConnectionTester", "Received response from node %d", Nodes[nodeId].address);
    }
    else
    {
        LOG_WARN("ConnectionTester", "No response received from node %d within timeout", Nodes[nodeId].address);
    }
}

void ConnectionTester::handleBroadcastResponse(Event e)
{
    uint8_t from = e.data1;
    int16_t rssi = e.data2;
    float snr = *reinterpret_cast<float *>(&e.data3);

    LOG_INFO("ConnectionTester", "Received broadcast response from node %d with RSSI %d and SNR %.2f", from, rssi, snr);
    addNewNode(from, rssi, snr);
}

void ConnectionTester::handleBroadcast(Event e)
{
    if(respondedToBroadcast)
    {
        LOG_DEBUG("ConnectionTester", "Already responded to a broadcast, ignoring this one");
        return;
    }
    uint8_t from = e.data1;
    LOG_INFO("ConnectionTester", "Received broadcast message from node %d", from);
    // Respond to broadcast with our own info with little delay to avoid collisions
    vTaskDelay(150 * LORA_ADDRESS / portTICK_PERIOD_MS);
    EventGroupHandle_t responseEvent = xEventGroupCreate();
    EventBits_t responseBit = 0x01;
    String response = Protocol::buildTestBrResponseMsg(LORA_ADDRESS);
    networkManager->sendToAndWait(from, response, responseEvent, responseBit);
    EventBits_t result = xEventGroupWaitBits(responseEvent, responseBit, pdTRUE, pdFALSE, pdMS_TO_TICKS(20000)); // wait for ACK just to prevent task from being deleted before response is sent
    if(result & responseBit)
    {
        respondedToBroadcast = true;
        LOG_DEBUG("ConnectionTester", "Received ACK for broadcast response to node %d", from);
    }
    else
    {
        LOG_WARN("ConnectionTester", "No ACK received for broadcast response to node %d within timeout", from);
    }

}

void ConnectionTester::handleDirectResponse(Event e)
{
    uint8_t from = e.data1;
    int16_t rssi = e.data2;
    float snr = *reinterpret_cast<float *>(&e.data3);
    uint8_t packetId = e.data4;
    LOG_INFO("ConnectionTester", "Received direct response from node %d with RSSI %d and SNR %.2f", from, rssi, snr);
    
    int nodeIndex = findNodeIndexByAddress(from);
    if (nodeIndex == -1)
    {
        LOG_ERROR("ConnectionTester", "Received direct response from unknown node %d", from);
        return;
    }
    if(Nodes[nodeIndex].lastresponse >= 10)
    {
        LOG_ERROR("ConnectionTester", "Too many responses received from node %d, overwriting old responses", from);
    }
    Nodes[nodeIndex].responses[Nodes[nodeIndex].lastresponse].packetId = packetId;
    Nodes[nodeIndex].responses[Nodes[nodeIndex].lastresponse].rssi = rssi;
    Nodes[nodeIndex].responses[Nodes[nodeIndex].lastresponse].snr = snr;
    Nodes[nodeIndex].responses[Nodes[nodeIndex].lastresponse].retryCount = 0; // not needed
    Nodes[nodeIndex].lastresponse = (Nodes[nodeIndex].lastresponse + 1) % 10;
    
}

void ConnectionTester::handleDirect(Event e)
{
    uint8_t from = e.data1;
    int16_t rssi = e.data2;
    float snr = *reinterpret_cast<float *>(&e.data3);
    LOG_INFO("ConnectionTester", "Received direct test message from node %d with RSSI %d and SNR %.2f", from, rssi, snr);
    respondedToBroadcast = false; 

    // Respond to direct test with our own info
    String response = Protocol::buildTestDrResponseMsg(LORA_ADDRESS, 1, 0, rssi, snr); // packetId, packetsReceived and retryCount are not needed in response
    networkManager->sendTo(from, response);
}

void ConnectionTester::createTestTask()
{
    LOG_DEBUG("ConnectionTester", "Creating connection test task...");
    xTaskCreate(
        [](void *pvParameters)
        {
            ConnectionTester *tester = static_cast<ConnectionTester *>(pvParameters);
            tester->testTask(pvParameters);
        },
        "ConnectionTestTask",
        8192,
        this,
        1,
        nullptr);
}

void ConnectionTester::testTask(void *pvParameters)
{
    bool broadcastLoop = true;
    bool directTestLoop = true;

    LOG_DEBUG("ConnectionTester", "Entering broadcast loop");
    while (broadcastLoop)
    {
        broadcastTestSearch();
        vTaskDelay(2000 * nodeCount / portTICK_PERIOD_MS);
        if (nodesFound >= nodeCount)
        {
            broadcastLoop = false;
            continue;
        }
    }
    if (nodesFound == nodeCount)
    {
        LOG_INFO("ConnectionTester", "All nodes found in broadcast loop");
    }
    else
    {
        LOG_INFO("ConnectionTester", "Broadcast loop ended, found %d out of %d nodes", nodesFound, nodeCount);
    }

    for (size_t i = 0; i < nodesFound; i++)
    {
        LOG_INFO("ConnectionTester", "Node %d: Address=%d, RSSI=%d, SNR=%.2f", i, Nodes[i].address, Nodes[i].rssi, Nodes[i].snr);
    }
    

    LOG_DEBUG("ConnectionTester", "broadcast done, switching to direct test mode  ");
    while (directTestLoop)
    {
        LOG_INFO("ConnectionTester", "Starting direct test loop with %d nodes", nodesFound);
        directTestLoop = false;
        for (int i = 0; i < nodesFound; i++)
        {
            LOG_INFO("ConnectionTester", "Testing connection to node %d at address %d", i, Nodes[i].address);
            sendTestToNode(i);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }

    LOG_DEBUG("ConnectionTester", "Direct test loop completed, printing results:");
    for (int i = 0; i < nodesFound; i++)
    {
        LOG_INFO("ConnectionTester", "Node %d at address %d received %d responses out of %d:", i, Nodes[i].address, Nodes[i].lastresponse, Nodes[i].generatedPackets);
        for (int j = 0; j < 10; j++)
        {
            if (Nodes[i].responses[j].packetId != 0)
            {
                LOG_INFO("ConnectionTester", "  Response %d: RSSI=%d, SNR=%.2f", j, Nodes[i].responses[j].rssi, Nodes[i].responses[j].snr);
            }
        }
    }
    
    vTaskDelete(nullptr);
}

int ConnectionTester::findNodeIndexByAddress(uint8_t address)
{
    for (int i = 0; i < nodesFound; i++)
    {
        if (Nodes[i].address == address)
        {
            return i;
        }
    }
    return -1;
}
