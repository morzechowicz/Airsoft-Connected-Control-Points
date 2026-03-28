// BaseComponent.h - IMPROVED VERSION
#ifndef BASE_COMPONENT_H
#define BASE_COMPONENT_H

#include <Arduino.h>
#include "EventBus.h"
#include "Hardware/HardwareManager.h"
#include "Network/NetworkManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../lib/Logging/LogManager.h"

#define BASECOMPONENT_MODE_TASK_STACK 8192  // Increased from 4096
#define BASECOMPONENT_MODE_TASK_PRIORITY 1

class BaseComponent {
public:
    BaseComponent(EventBus *eventBus, HardwareManager *hardwareManager, NetworkManager *networkManager);
    virtual ~BaseComponent();

    // Network callback
    virtual void onDataReceived(const char data[128], size_t length) {}

    // Mode lifecycle - override in derived modes
    virtual void enterMode() {}
    virtual void exitMode() {}

    // The main mode loop - must be implemented by derived classes
    virtual void run() = 0;

    // Task management
    bool startModeTask(const char* name = "ModeTask", 
                      UBaseType_t priority = BASECOMPONENT_MODE_TASK_PRIORITY, 
                      uint32_t stack = BASECOMPONENT_MODE_TASK_STACK);
    void stopModeTask();
    
    // Check if task is running
    bool isTaskRunning() const { return taskRunning; }

protected:
    EventBus *eventBus;
    HardwareManager *hardwareManager;
    NetworkManager *networkManager;

    TaskHandle_t modeTaskHandle;
    volatile bool taskRunning;  // Flag for graceful shutdown

    // Static entry point for FreeRTOS task
    static void modeTaskEntry(void* pv) {
        BaseComponent* self = static_cast<BaseComponent*>(pv);
        if (!self) {
            LOG_ERROR("BASE_COMPONENT", "Null component in task entry!");
            vTaskDelete(NULL);
            return;
        }
        
        LOG_INFO("BASE_COMPONENT", "Task entry - calling enterMode()");
        self->enterMode();
        
        LOG_INFO("BASE_COMPONENT", "Task entry - calling run()");
        self->run();
        
        LOG_INFO("BASE_COMPONENT", "Task entry - calling exitMode()");
        self->exitMode();
        
        // Clear handle before deleting
        self->modeTaskHandle = nullptr;
        self->taskRunning = false;
        
        LOG_INFO("BASE_COMPONENT", "Task entry - deleting task");
        vTaskDelete(NULL);
        self->~BaseComponent();  // Ensure destructor is called
    }
};

#endif // BASE_COMPONENT_H