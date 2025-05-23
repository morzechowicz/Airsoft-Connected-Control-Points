#include <LoRaHandler.h>

LoRaHandler::LoRaHandler(LoRaCom &commManager, LoRaMsgHandler &msgHandler)
    : commManager(commManager), msgHandler(msgHandler) {}

void LoRaHandler::begin() {
    commManager.begin();
}

void LoRaHandler::loop() {
    String msg = commManager.reciveMsg();
    if (!msg.isEmpty()) {
        msgHandler.handleMessage(msg);
    }
}