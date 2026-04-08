#include "BleSetup.h"

void BleSetup::BleStart()
{
    NimBLEDevice::deinit(true);
    // Initialize BLE
    LOG_INFO("BLE", "Initializing BLE...");
    char deviceName[32];
    #if NODE_TYPE == CAPTURE_POINT
        snprintf(deviceName, sizeof(deviceName), "SPAS_CP_%d", LORA_ADDRESS);
    #elif NODE_TYPE == INFORMATION
        snprintf(deviceName, sizeof(deviceName), "SPAS_Info_%d", LORA_ADDRESS);
    #elif NODE_TYPE == BLEToLoRa
        snprintf(deviceName, sizeof(deviceName), "SPAS_Fwd_%d", LORA_ADDRESS);
    #else
        #error "Unknown NODE_TYPE"
    #endif
    LOG_INFO("BLE", "Device Name: %s", deviceName);
    Serial.printf("LORA_ADDRESS raw value: %d\n", LORA_ADDRESS);
    Serial.printf("deviceName: '%s'\n", deviceName);

    vTaskDelay(200); // Small delay to ensure BLE stack is ready after deinit
    NimBLEDevice::init(deviceName);
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
    NimBLEAdvertisementData advertisementData;
    advertisementData.setName(deviceName);
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponseData(advertisementData);
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