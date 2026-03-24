#include "EventBus.h"

EventBus::EventBus() : queueHead(0), queueTail(0) {
}

void EventBus::subscribe(EventType eventName, EventCallback callback) {
    if (eventName >= 0 && eventName < EVENT_MAX) {
        listeners[eventName].push_back(callback);
    }
}

void EventBus::unsubscribe(EventType eventName) {
    if (eventName >= 0 && eventName < EVENT_MAX) {
        listeners[eventName].clear();
        Serial.print("Unsubscribed all listeners from event: ");
        Serial.println(eventName);
    }
}

// ============================================
// ONE PUBLISH FUNCTION WITH DEFAULT PARAMETERS
// ============================================

// here just data 1,2,3,4,5
void EventBus::publish(EventType eventName, int data1, int data2, int data3, int data4, int data5) {
    int nextTail = (queueTail + 1) % QUEUE_SIZE;
    
    if (nextTail == queueHead) {
        // Queue full
        Serial.println("WARNING: Event queue full!");
        return;
    }
    
    // Create event with all data
    eventQueue[queueTail].type = eventName;
    eventQueue[queueTail].data1 = data1;
    eventQueue[queueTail].data2 = data2;
    eventQueue[queueTail].data3 = data3;
    eventQueue[queueTail].data4 = data4;
    eventQueue[queueTail].data5 = data5;
    
    queueTail = nextTail;
}
// here with team points array max 3 data
void EventBus::publish(EventType eventName, int data1, int data2, int data3, int len, NodeState teamPoints[10])
{
    int nextTail = (queueTail + 1) % QUEUE_SIZE;
    
    if (nextTail == queueHead) {
        // Queue full
        Serial.println("WARNING: Event queue full!");
        return;
    }
    
    // Create event with all data
    eventQueue[queueTail].type = eventName;
    eventQueue[queueTail].data1 = data1;
    eventQueue[queueTail].data2 = data2;
    eventQueue[queueTail].data3 = data3;
    eventQueue[queueTail].data4 = len;
    for (int i = 0; i < 10; i++) {
        eventQueue[queueTail].teamPoints[i] = teamPoints[i];
    }
    
    queueTail = nextTail;
}

void EventBus::publish(EventType eventName, int data1, String msg)
{
    int nextTail = (queueTail + 1) % QUEUE_SIZE;
    
    if (nextTail == queueHead) {
        // Queue full
        Serial.println("WARNING: Event queue full!");
        return;
    }
    
    // Create event with all data
    eventQueue[queueTail].type = eventName;
    eventQueue[queueTail].data1 = data1;
    eventQueue[queueTail].message = msg;
    
    queueTail = nextTail;
}

void EventBus::processEvents()
{
    while (queueHead != queueTail) {
        Event currentEvent = eventQueue[queueHead];
        queueHead = (queueHead + 1) % QUEUE_SIZE;
        
        for (auto& cb : listeners[currentEvent.type]) {
            if (cb) {
                cb(currentEvent);
            }
        }
    }
}