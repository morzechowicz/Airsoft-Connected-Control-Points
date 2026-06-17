#ifndef CONNECTION_TESTER_H
#define CONNECTION_TESTER_H

#include <Arduino.h>
#include "NetworkManager.h"
#include "Hardware/HardwareManager.h"
#include "../lib/Logging/LogManager.h"
#include "EventBus.h"
#include "Protocol.h"

#define NUMBER_OF_PROBES 5
#define WAITING_TIME 700 // ms

struct responseProbe
{
    int   probeId        = 0;
    int   rssi           = 0;
    float snr            = 0.0f;
    bool  responseStatus = false; 
};

class ConnectionTester
{
public:
    ConnectionTester(NetworkManager *networkManager, HardwareManager *hardwareManager, EventBus *eventBus);

private:
    NetworkManager *networkManager;
    HardwareManager *hardwareManager;
    EventBus *eventBus;
    responseProbe probes[NUMBER_OF_PROBES];

    xQueueHandle responseQueue;
    xTaskHandle testConTask;
    void testConnectionTask(uint8_t target);

    void testConnection(Event e);
    void testConnectionCommand(Event e);
    void testConnectionResponse(Event e);

    void sendProbe(uint8_t targetId, uint8_t probeId);
};

#endif // CONNECTION_TESTER_H