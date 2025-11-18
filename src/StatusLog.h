#ifndef STATUSLOG_H
#define STATUSLOG_H

#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>
#include "Clocker.h"
#include "event_codes.h"


struct logEntry
{
    u_int32_t timestamp;
    uint8_t event_id;
    uint8_t node_id;
};

class StatusLog
{
public:
    StatusLog(Clocker &clock) : gameClock(clock) {}
    void addLogEntry(uint8_t event_id, uint8_t node_id = NODE_ID);
    void printLog();
    void clearLog();
    String getLogAsString();
private:
    static const int MAX_LOG_ENTRIES = 1000;
    logEntry logEntries[MAX_LOG_ENTRIES];
    int currentIndex = 0;
    Clocker &gameClock;
};
#endif // STATUSLOG_H