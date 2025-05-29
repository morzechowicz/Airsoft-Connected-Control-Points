#include "AckList.h"

void AckList::add(LoRaAckMessage value)
{
    data.push_back(value); // Add the integer to the end of the vector
}

void AckList::remove(LoRaAckMessage *value)
{
    // Find the value in the vector
    if (value == nullptr)
    {
        Serial.println("Cannot remove: Pointer is null!");
        return;
    }

    auto it = std::find_if(data.begin(), data.end(), [value](const LoRaAckMessage &msg)
                           {
                               return &msg == value; // Compare the address of the object
                           });

    if (it != data.end())
    {
        Serial.print("Removing message with SeqNum: ");
        Serial.println(it->getSeqNum());
        data.erase(it);
    }
    else
    {
        Serial.println("Message not found in list!");
    }
}

bool AckList::contains(LoRaAckMessage &value)
{
    // Check if the value exists in the vector
    return std::find(data.begin(), data.end(), value) != data.end();
}

LoRaAckMessage *AckList::get(int index)
{
    return &data.at(index);
}

LoRaAckMessage *AckList::getBySeqNum(int seqNum)
{
    for (auto &msg : data)
    {
        if (msg.getSeqNum() == seqNum)
        {
            return &msg;
        }
    }
    return nullptr; // Return nullptr if not found
}

void AckList::print()
{
    Serial.println("Container content: FIX THE PRINT DUMBASS");
}

size_t AckList::size() const
{
    return data.size(); // Return the number of elements in the vector
}