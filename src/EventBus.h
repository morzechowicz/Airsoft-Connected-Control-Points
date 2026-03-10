#ifndef EVENTBUS_H
#define EVENTBUS_H

#include "EventType.h"
#include <functional>
#include <vector>
#include <Arduino.h>

class EventBus
{
public:
    typedef std::function<void(Event)> EventCallback;

private:
    static const int MAX_LISTENERS = 5;
    std::vector<EventCallback> listeners[EVENT_MAX];

    static const int QUEUE_SIZE = 20;
    Event eventQueue[QUEUE_SIZE];
    int queueHead;
    int queueTail;

public:
    EventBus();
    void subscribe(EventType eventName, EventCallback callback);
    void unsubscribe(EventType eventName);

    // ONE publish with default parameters!
    void publish(EventType eventName,
                 int data1 = 0,
                 int data2 = 0,
                 int data3 = 0,
                 int data4 = 0,
                 int data5 = 0);

    void processEvents();
};

#endif // EVENTBUS_H