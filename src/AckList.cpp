#include "AckList.h"

void AckList::add(LoRaAckMessage value) {
    data.push_back(value); // Add the integer to the end of the vector
}

void AckList::remove(LoRaAckMessage value) {
    // Find the value in the vector
    auto it = std::find(data.begin(), data.end(), value);
    if (it != data.end()) {
        data.erase(it); // Remove the value if found
    }
}

bool AckList::contains(LoRaAckMessage value) {
    // Check if the value exists in the vector
    return std::find(data.begin(), data.end(), value) != data.end();
}

LoRaAckMessage AckList::getBySeqNum(uint16_t seqNum)
{
    for (auto& msg : data) {
        if (msg.getSeqNum() == seqNum) {
            return msg;
        }
    }
    return LoRaAckMessage(); // Return a default-constructed message if not found
}

void AckList::print()
{
    Serial.println("Container content: FIX THE PRINT DUMBASS");
}

size_t AckList::size() const {
    return data.size(); // Return the number of elements in the vector
}