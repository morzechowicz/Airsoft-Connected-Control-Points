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
        Serial.println("Task already running!");
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
        Serial.print("Task started: ");
        Serial.println(name);
        return true;
    } else {
        Serial.print("Task creation failed: ");
        Serial.println(name);
        taskRunning = false;
        modeTaskHandle = nullptr;
        return false;
    }
}

void BaseComponent::stopModeTask() {
    if (modeTaskHandle == nullptr) return;
    
    Serial.println("Stopping task...");
    
    // Signal task to stop
    taskRunning = false;
    
    // Give task time to clean up gracefully
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Force delete if still running
    if (modeTaskHandle != nullptr) {
        vTaskDelete(modeTaskHandle);
        modeTaskHandle = nullptr;
    }
    
    Serial.println("Task stopped");
}