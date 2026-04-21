#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <vector>
#include <functional>

QueueHandle_t bleLogQueue;

// Log levels
enum LogLevel
{
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARN = 2,
    LOG_ERROR = 3,
    LOG_NONE = 4 // Disable all logging
};

// Output flags
#define LOG_OUTPUT_SERIAL 0x01
#define LOG_OUTPUT_BLE 0x02
#define LOG_OUTPUT_LORA 0x04

// Colors for serial output (ANSI escape codes)
#define LOG_COLOR_RESET "\033[0m"
#define LOG_COLOR_DEBUG "\033[36m" // Cyan
#define LOG_COLOR_INFO "\033[32m"  // Green
#define LOG_COLOR_WARN "\033[33m"  // Yellow
#define LOG_COLOR_ERROR "\033[31m" // Red

class LogManager
{
public:
    static LogManager &getInstance()
    {
        static LogManager instance;
        return instance;
    }

    // Initialization
    void begin(LogLevel level = LOG_INFO, uint8_t outputs = LOG_OUTPUT_SERIAL);

    // ble queue task
    void bleLogTask(void *pvParameters);
    void createBleLogTask();

    // Configuration
    void setLogLevel(LogLevel level);
    void setOutputs(uint8_t outputs);
    void enableColors(bool enable);
    void setTimestamps(bool enable);
    void setTags(bool enable);
    void setBLEStatus(bool connected);

    // Output callbacks (for BLE/LoRa)
    void setBLECallback(std::function<void(const char *)> callback);
    void setLoRaCallback(std::function<void(const char *)> callback);

    // Main logging methods
    void log(LogLevel level, const char *tag, const char *format, ...);
    void logRaw(const char *format, ...); // No formatting, just output

    // Convenience methods
    void debug(const char *tag, const char *format, ...);
    void info(const char *tag, const char *format, ...);
    void warn(const char *tag, const char *format, ...);
    void error(const char *tag, const char *format, ...);

    // Print methods (drop-in Serial replacement)
    void print(const char *str);
    void print(int val);
    void print(uint8_t val, int format = DEC);
    void println(const char *str);
    void println();
    void printf(const char *format, ...);

private:
    LogManager();
    ~LogManager();

    // Prevent copying
    LogManager(const LogManager &) = delete;
    LogManager &operator=(const LogManager &) = delete;

    void output(const char *message, uint8_t outputs);
    const char *getLevelColor(LogLevel level);
    const char *getLevelString(LogLevel level);

    LogLevel currentLevel;
    uint8_t outputFlags;
    bool useColors;
    bool useTimestamps;
    bool useTags;
    bool BLEConnected;

    SemaphoreHandle_t logMutex;

    std::function<void(const char *)> bleCallback;
    std::function<void(const char *)> loraCallback;

    char buffer[512]; // Shared buffer for formatting
};

// Global convenience macros
#define LOG LogManager::getInstance()

// Conditional logging macros (compile-time removal)
#ifdef LOG_ENABLE_DEBUG
#define LOGD(tag, ...) LOG.debug(tag, __VA_ARGS__)
#else
#define LOGD(tag, ...)
#endif

#ifdef LOG_ENABLE_INFO
#define LOGI(tag, ...) LOG.info(tag, __VA_ARGS__)
#else
#define LOGI(tag, ...)
#endif

#ifdef LOG_ENABLE_WARN
#define LOGW(tag, ...) LOG.warn(tag, __VA_ARGS__)
#else
#define LOGW(tag, ...)
#endif

#ifdef LOG_ENABLE_ERROR
#define LOGE(tag, ...) LOG.error(tag, __VA_ARGS__)
#else
#define LOGE(tag, ...)
#endif

// Always-on macros (runtime level check)
#define LOG_DEBUG(tag, ...) LOG.debug(tag, __VA_ARGS__)
#define LOG_INFO(tag, ...) LOG.info(tag, __VA_ARGS__)
#define LOG_WARN(tag, ...) LOG.warn(tag, __VA_ARGS__)
#define LOG_ERROR(tag, ...) LOG.error(tag, __VA_ARGS__)

#endif // LOG_MANAGER_H