#ifndef CONNECTION_TESTER_H
#define CONNECTION_TESTER_H

#include <Arduino.h>
#include "NetworkManager.h"
#include "Hardware/HardwareManager.h"
#include "../lib/Logging/LogManager.h"
#include "EventBus.h"
#include "Protocol.h"

class ConnectionTester
{
public:
    ConnectionTester(NetworkManager* networkManager, HardwareManager* hardwareManager, EventBus* eventBus);

private:
    NetworkManager *networkManager;
    HardwareManager *hardwareManager;
    EventBus *eventBus;

    void testConnection(Event e);
    void testConnectionCommand(Event e);
    void testConnectionResponse(Event e);
};

#endif // CONNECTION_TESTER_H