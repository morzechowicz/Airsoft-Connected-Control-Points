#include "LoRaAckMessage.h"

LoRaAckMessage::LoRaAckMessage()
{
}

LoRaAckMessage::LoRaAckMessage(const String &message, int seqNum) : msg(message), seqNum(seqNum), retryCount(0), interval(0), lastRetry(0), recived(false)
{
}

bool LoRaAckMessage::operator==(const LoRaAckMessage &other) const {
    return this->seqNum == other.seqNum;
}

void LoRaAckMessage::increamentRetry()
{
    retryCount = retryCount + 1;
    Serial.println("Retry number");
    Serial.println(retryCount);
}
