// BaseComponent.cpp - IMPROVED VERSION
#include "BaseComponent.h"

BaseComponent::BaseComponent(EventBus *eventBus, HardwareManager *hardwareManager, NetworkManager *networkManager)
    : eventBus(eventBus), 
      hardwareManager(hardwareManager), 
      networkManager(networkManager), 
      modeTaskHandle(nullptr),
      taskRunning(false)
{
}

BaseComponent::~BaseComponent()
{
    stopModeTask();
}

bool BaseComponent::startModeTask(const char* name, UBaseType_t priority, uint32_t stack) {
    if (modeTaskHandle != nullptr) {
        LOG_INFO("BASE_COMPONENT", "Task already running: %s", name);
        return false;
    }

    taskRunning = true;

    BaseType_t res = xTaskCreate(
        &BaseComponent::modeTaskEntry,
        name ? name : "ModeTask",
        (configSTACK_DEPTH_TYPE)stack / sizeof(StackType_t),
        this,
        priority,
        &modeTaskHandle
    );

    if (res == pdPASS) {
        LOG_INFO("BASE_COMPONENT", "Task started: %s", name);
        return true;
    } else {
        LOG_ERROR("BASE_COMPONENT", "Task creation failed: %s", name);
        taskRunning = false;
        modeTaskHandle = nullptr;
        return false;
    }
}

void BaseComponent::stopModeTask() {
    if (modeTaskHandle == nullptr) return;
    
    LOG_INFO("BASE_COMPONENT", "Stopping task...");
    
    // Signal task to stop
    taskRunning = false;
    
    // Give task time to clean up gracefully
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Force delete if still running
    if (modeTaskHandle != nullptr) {
        vTaskDelete(modeTaskHandle);
        modeTaskHandle = nullptr;
    }
    
    LOG_INFO("BASE_COMPONENT", "Task stopped");
}