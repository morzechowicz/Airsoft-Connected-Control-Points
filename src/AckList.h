#ifndef ACK_LIST_H
#define ACK_LIST_H

#include <vector>
#include <algorithm> 
#include <iostream>  
#include "LoRaAckMessage.h"

class AckList {
public:
    void add(LoRaAckMessage value);      
    void remove(LoRaAckMessage value);   
    bool contains(LoRaAckMessage value); 
    LoRaAckMessage get(int index) const { return data.at(index); } 
    LoRaAckMessage getBySeqNum(uint16_t seqNum);
    void print();             
    size_t size() const;     
private:
    std::vector<LoRaAckMessage> data;    
};

#endif