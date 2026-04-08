#include "BleSetup.h"

void BleSetup::BleStart()
{
    // Initialize BLE
    LOG_INFO("BLE", "Initializing BLE...");
    #if NODE_TYPE == CAPTURE_POINT
    String deviceName = "SPAS_CP_" + String(LORA_ADDRESS);
    #elif NODE_TYPE == INFORMATION
    String deviceName = "SPAS_Info_" + String(LORA_ADDRESS);
    #elif NODE_TYPE == FORWARDER
    String deviceName = "SPAS_Fwd_" + String(LORA_ADDRESS);  
    #else
    #error "Unknown NODE_TYPE, please define it as CAPTURE_POINT, INFORMATION or FORWARDER"
    #endif
    NimBLEDevice::init(deviceName.c_str());
    pServer = NimBLEDevice::createServer();
    bleServer = new BleServer();
    pServer->setCallbacks(bleServer);

    // Create BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create BLE Characteristic (read/write)
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    pCharacteristic->setValue("Hello from ESP32 (NimBLE)");
    pCharacteristic->setCallbacks(new BleCallback(eventBus, configHandler));

    // Start the service
    LOG_INFO("BLE", "BLE Service started");
    vTaskDelay(200);
    // Start advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setName(deviceName.c_str());
    pAdvertising->start();
    LOG_INFO("BLE", "BLE Ready! Waiting for connections...");
}

BleSetup::~BleSetup()
{
}

void BleSetup::sendMessage(const String& message)
{
    if (pCharacteristic) {
        pCharacteristic->setValue(message.c_str());
        pCharacteristic->notify();
    }
}