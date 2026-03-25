/*
 * LogManager - Implementation
 */

#include "LogManager.h"
#include <stdarg.h>

LogManager::LogManager() 
    : currentLevel(LOG_INFO),
      outputFlags(LOG_OUTPUT_SERIAL),
      useColors(true),
      useTimestamps(true),
      useTags(true),
      bleCallback(nullptr),
      loraCallback(nullptr)
{
    logMutex = xSemaphoreCreateMutex();
}

LogManager::~LogManager() {
    if (logMutex) {
        vSemaphoreDelete(logMutex);
    }
}

void LogManager::begin(LogLevel level, uint8_t outputs) {
    currentLevel = level;
    outputFlags = outputs;
    
    if (outputFlags & LOG_OUTPUT_SERIAL) {
        Serial.begin(115200);
        delay(100);
    }
}

void LogManager::setLogLevel(LogLevel level) {
    currentLevel = level;
}

void LogManager::setOutputs(uint8_t outputs) {
    outputFlags = outputs;
}

void LogManager::enableColors(bool enable) {
    useColors = enable;
}

void LogManager::setTimestamps(bool enable) {
    useTimestamps = enable;
}

void LogManager::setTags(bool enable) {
    useTags = enable;
}

void LogManager::setBLECallback(std::function<void(const char*)> callback) {
    bleCallback = callback;
}

void LogManager::setLoRaCallback(std::function<void(const char*)> callback) {
    loraCallback = callback;
}

const char* LogManager::getLevelColor(LogLevel level) {
    if (!useColors) return "";
    
    switch (level) {
        case LOG_DEBUG: return LOG_COLOR_DEBUG;
        case LOG_INFO:  return LOG_COLOR_INFO;
        case LOG_WARN:  return LOG_COLOR_WARN;
        case LOG_ERROR: return LOG_COLOR_ERROR;
        default:        return LOG_COLOR_RESET;
    }
}

const char* LogManager::getLevelString(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO ";
        case LOG_WARN:  return "WARN ";
        case LOG_ERROR: return "ERROR";
        default:        return "?????";
    }
}

void LogManager::log(LogLevel level, const char* tag, const char* format, ...) {
    // Check log level
    if (level < currentLevel) return;
    
    // Acquire mutex
    if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return; // Failed to acquire mutex
    }
    
    // Build message
    int offset = 0;
    
    // Timestamp
    if (useTimestamps) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, 
                          "[%lu] ", millis());
    }
    
    // Color + Level
    if (useColors && (outputFlags & LOG_OUTPUT_SERIAL)) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                          "%s[%s]%s ", 
                          getLevelColor(level),
                          getLevelString(level),
                          LOG_COLOR_RESET);
    } else {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                          "[%s] ", getLevelString(level));
    }
    
    // Tag
    if (useTags && tag) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                          "[%s] ", tag);
    }
    
    // Message
    va_list args;
    va_start(args, format);
    offset += vsnprintf(buffer + offset, sizeof(buffer) - offset, format, args);
    va_end(args);
    
    // Newline
    if (offset < sizeof(buffer) - 1) {
        buffer[offset++] = '\n';
        buffer[offset] = '\0';
    }
    
    // Output to all enabled channels
    output(buffer, outputFlags);
    
    xSemaphoreGive(logMutex);
}

void LogManager::logRaw(const char* format, ...) {
    if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    output(buffer, outputFlags);
    
    xSemaphoreGive(logMutex);
}

void LogManager::debug(const char* tag, const char* format, ...) {
    if (LOG_DEBUG < currentLevel) return;
    
    va_list args;
    va_start(args, format);
    
    char msgBuffer[256];
    vsnprintf(msgBuffer, sizeof(msgBuffer), format, args);
    va_end(args);
    
    log(LOG_DEBUG, tag, "%s", msgBuffer);
}

void LogManager::info(const char* tag, const char* format, ...) {
    if (LOG_INFO < currentLevel) return;
    
    va_list args;
    va_start(args, format);
    
    char msgBuffer[256];
    vsnprintf(msgBuffer, sizeof(msgBuffer), format, args);
    va_end(args);
    
    log(LOG_INFO, tag, "%s", msgBuffer);
}

void LogManager::warn(const char* tag, const char* format, ...) {
    if (LOG_WARN < currentLevel) return;
    
    va_list args;
    va_start(args, format);
    
    char msgBuffer[256];
    vsnprintf(msgBuffer, sizeof(msgBuffer), format, args);
    va_end(args);
    
    log(LOG_WARN, tag, "%s", msgBuffer);
}

void LogManager::error(const char* tag, const char* format, ...) {
    if (LOG_ERROR < currentLevel) return;
    
    va_list args;
    va_start(args, format);
    
    char msgBuffer[256];
    vsnprintf(msgBuffer, sizeof(msgBuffer), format, args);
    va_end(args);
    
    log(LOG_ERROR, tag, "%s", msgBuffer);
}

void LogManager::output(const char* message, uint8_t outputs) {
    // Serial output
    if (outputs & LOG_OUTPUT_SERIAL) {
        Serial.print(message);
    }
    
    // BLE output
    if ((outputs & LOG_OUTPUT_BLE) && bleCallback) {
        bleCallback(message);
    }
    
    // LoRa output
    if ((outputs & LOG_OUTPUT_LORA) && loraCallback) {
        loraCallback(message);
    }
}

// Drop-in Serial replacements
void LogManager::print(const char* str) {
    logRaw("%s", str);
}

void LogManager::print(int val) {
    logRaw("%d", val);
}

void LogManager::print(uint8_t val, int format) {
    if (format == HEX) {
        logRaw("%02X", val);
    } else {
        logRaw("%d", val);
    }
}

void LogManager::println(const char* str) {
    logRaw("%s\n", str);
}

void LogManager::println() {
    logRaw("\n");
}

void LogManager::printf(const char* format, ...) {
    if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    output(buffer, outputFlags);
    
    xSemaphoreGive(logMutex);
}