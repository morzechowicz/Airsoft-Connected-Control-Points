#include "BleSetup.h"

void BleSetup::BleStart()
{
    // Initialize BLE
    Serial.println("Initializing BLE...");
    String deviceName = "LoRaCP_" + String(myNodeId);
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
    pService->start();
    Serial.println("BLE Service started");
    vTaskDelay(200);
    // Start advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setName(deviceName.c_str());
    pAdvertising->start();
    Serial.println("BLE Ready! Waiting for connections...");
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