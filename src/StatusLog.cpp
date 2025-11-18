#include "StatusLog.h"

void StatusLog::addLogEntry(uint8_t event_id, uint8_t node_id)
{
    if (currentIndex < MAX_LOG_ENTRIES)
    {
        logEntries[currentIndex].timestamp = gameClock.getElapsedTimeInSeconds();
        logEntries[currentIndex].event_id = event_id;
        logEntries[currentIndex].node_id = node_id;
        currentIndex++;
    }
}

void StatusLog::printLog()
{
    for (int i = 0; i < currentIndex; i++)
    {
        Serial.print("Log Entry ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(logEntries[i].timestamp);
        Serial.print(", ");
        Serial.println(logEntries[i].event_id);
        Serial.print(", ");
        Serial.println(logEntries[i].node_id);
    }
}

void StatusLog::clearLog()
{
    currentIndex = 0;
}

String StatusLog::getLogAsString()
{
    String logString;
    for (int i = 0; i < currentIndex; i++)
    {
        logString += "L";
        logString += String(i);
        logString += ":";
        logString += String(logEntries[i].timestamp);
        logString += ":";
        logString += String(logEntries[i].event_id);
        logString += ":";
        logString += String(logEntries[i].node_id);
        logString += "\n";
    }
    return logString;
}
