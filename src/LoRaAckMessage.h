#ifndef LORA_ACK_MESSAGE_H
#define LORA_ACK_MESSAGE_H

#include <Arduino.h>

class LoRaAckMessage
{
public:
    LoRaAckMessage();
    LoRaAckMessage(const String &message, int seqNum);

    bool operator==(const LoRaAckMessage &other) const;
    void increamentRetry();

    inline const String &getMessage() const { return msg; }
    inline int getSeqNum() const { return seqNum; }
    inline int getRetryCount() const { return retryCount; }
    inline int getInterval() const { return interval; }
    inline int getLastRetry() const { return lastRetry; }
    inline bool getRecived() const { return recived; }

    inline void setMessage(const String &message) { msg = message; }
    inline void setSeqNum(int num) { seqNum = num; }
    inline void setRetryCount(int count) { retryCount = count; }
    inline void setInterval(int val) { interval = val; }
    inline void setLastRetry(int val) { lastRetry = val; }
    inline void setRecived(bool val) { recived = val;}
private:
    String msg;
    int seqNum;
    int retryCount;
    int interval;
    int lastRetry;
    bool recived;
};

#endif // LORA_ACK_MESSAGE_H