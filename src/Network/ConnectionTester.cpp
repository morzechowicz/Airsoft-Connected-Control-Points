#include "ConnectionTester.h"

ConnectionTester::ConnectionTester(NetworkManager *networkManager, HardwareManager *hardwareManager, EventBus *eventBus) : networkManager(networkManager),
                                                                                                                           hardwareManager(hardwareManager),
                                                                                                                           eventBus(eventBus)
{
    eventBus->subscribe(TEST_BEGIN, [this](Event e)
                        { testConnection(e); });
    eventBus->subscribe(TEST_DIRECT, [this](Event e)
                        { testConnectionCommand(e); });
    eventBus->subscribe(TEST_DR_RESPONSE, [this](Event e)
                        { testConnectionResponse(e); });

    responseQueue = xQueueCreate(NUMBER_OF_PROBES, sizeof(responseProbe));
}

void ConnectionTester::testConnectionTask(uint8_t target)
{
    LOG_DEBUG("CONNECTION_TESTER", "Starting connection testing task");

    for (uint8_t probe = 0; probe < NUMBER_OF_PROBES; probe++)
    {
        responseProbe respProbe{};
        LOG_DEBUG("CONNECTION_TESTER", "sending probe %d", probe);
        respProbe.probeId = probe;
        LOG_DEBUG("sizeof(responseProbe)", "%d", sizeof(responseProbe));
        sendProbe(target, respProbe.probeId);
        vTaskDelay(pdMS_TO_TICKS(1500)); // dramatic pause
        if (xQueueReceive(responseQueue, &respProbe, pdMS_TO_TICKS(WAITING_TIME)))
        {
            LOG_DEBUG("CONNECTION_TESTER", "recived response %d status %d", probe, respProbe.responseStatus);
            probes[probe] = respProbe;
        }
        else
        {
            LOG_DEBUG("CONNECTION_TESTER", "lost response %d status %d", probe, respProbe.responseStatus);
            probes[probe] = respProbe;
        }
    }

    LOG_DEBUG("CONNECTION_TESTER", "testing complete");
    // calculate signal quality
    uint8_t packetsConfimerd = 0;
    float snrSum = 0;
    int rssiSum = 0;
    for (int i = 0; i < NUMBER_OF_PROBES; i++)
    {
        if (!probes[i].responseStatus)
        {
            LOG_DEBUG("CONNECTION_TESTER", "probe %d failed", i);
            continue;
        }
        packetsConfimerd++;
        rssiSum += probes[i].rssi;
        snrSum += probes[i].snr;
    }
    if (packetsConfimerd == 0)
    {
        LOG_DEBUG("CONNECTION_TESTER", "Packet loss 100%");
        return;
    }
    int rssiAvg = rssiSum / packetsConfimerd;
    float snrAvg = snrSum / packetsConfimerd;

    LOG_INFO("CONNECTION_TESTER", "Packet loss %d", NUMBER_OF_PROBES - packetsConfimerd);
    LOG_INFO("CONNECTION_TESTER", "rssi %d", rssiAvg);
    LOG_INFO("CONNECTION_TESTER", "snr %f", snrAvg);
}

void ConnectionTester::testConnection(Event e)
{
    uint8_t targetNode = e.data1;
    uint16_t packetId = e.data2;
    LOG_INFO("CONNECTION_TESTER", "Starting test connection with node %d", targetNode);

    struct TaskParams
    {
        ConnectionTester *tester;
        uint8_t targetId;
    };
    auto *params = new TaskParams{this, targetNode};

    xTaskCreate(
        [](void *param)
        {
            auto *p = static_cast<TaskParams *>(param);
            ConnectionTester *tester = p->tester;
            uint8_t target = p->targetId;

            delete p; // free immediately, before doing any work

            tester->testConnectionTask(target);

            // Clear the handle on the manager before self-deleting
            tester->testConTask = nullptr;
            vTaskDelete(NULL);
        },
        "conTest", 8192, params, 1, &testConTask);
}

void ConnectionTester::testConnectionCommand(Event e)
{
    uint8_t sourceId = e.data1;
    int16_t rssi = e.data2;
    float snr = e.data3;
    uint8_t packetId = e.data4;
    LOG_INFO("CONNECTION_TESTER", "Received test connection command from node %d with RSSI %d and SNR %.2f", sourceId, rssi, snr);
    if (networkManager)
    {
        String msg = Protocol::buildTestDrResponseMsg(LORA_ADDRESS, packetId, 1, rssi, snr);
        networkManager->sendToUnreliable(sourceId, msg);
        LOG_DEBUG("CONNECTION_TESTER", "Sent test connection response to node %d", sourceId);
    }
}

void ConnectionTester::testConnectionResponse(Event e)
{
    LOG_INFO("CONNECTION_TESTER", "Received test connection probe %d from node %d with RSSI %d and SNR %.2f", e.data2, e.data1, e.data3, e.data4);
    responseProbe response;
    response.rssi = e.data2;
    response.snr = e.data3;
    response.probeId = e.data2;
    response.responseStatus = true;
    LOG_DEBUG("sizeof(responseProbe)", "%d", sizeof(responseProbe));
    if (!xQueueSend(responseQueue, &response, pdMS_TO_TICKS(10000)))
    {
        LOG_ERROR("CONNECTION_TESTER", "send responseQueue failed");
    };
}

void ConnectionTester::sendProbe(uint8_t targetId, uint8_t probeId)
{
    if (networkManager)
    {
        String msg = Protocol::buildTestDrMsg(LORA_ADDRESS, probeId); // i dont think i need second parameter
        networkManager->sendToUnreliable(targetId, msg);
        LOG_DEBUG("CONNECTION_TESTER", "Sent test connection message to node %d", targetId);
    }
}
