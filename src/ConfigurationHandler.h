#ifndef CONFIGURATION_HANDLER_H
#define CONFIGURATION_HANDLER_H

#include <Arduino.h>
#include "EventBus.h"
#include "Protocol.h"
#include "Config.h"

class ConfigurationHandler
{
public:
    ConfigurationHandler(EventBus& eb);
    
    // Main entry point - handles ANY config command from ANY source
    bool handleCommand(const Message& command);

    void handleSystemMessage();

    // Get result of last command (for responses)
    String getLastResult() const { return lastResult; }
    bool wasSuccessful() const { return lastSuccess; }
    
private:
    EventBus& eventBus;
    
    String lastResult;
    bool lastSuccess;
    
    NodeState nodeState[10];

    // Individual command handlers
    void handleSystemMessage(const String& cmd, const String params[], int paramCount);
    void handleConfigurationMessage(const String& cmd, const String params[], int paramCount);
    void handleGameMessage(const String& cmd, const String params[], int paramCount);

    
    // // Helpers
    // void setResult(bool success, const String& message);
};

#endif //CONFIGURATION_HANDLER_H
