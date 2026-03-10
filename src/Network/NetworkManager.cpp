#include "NetworkManager.h"

NetworkManager *NetworkManager::s_instance = nullptr;

NetworkManager::NetworkManager(EventBus &eventBus, ConfigurationHandler &confHandler) : eventBus(eventBus), confHandler(confHandler)
{
    role = ROLE_UNDEFINED;
    s_instance = this;
}

void NetworkManager::begin()
{

    myAddress = LORA_ADDRESS;
    networkReady = true;
    SComm.begin(LORA_ADDRESS, LORA_FREQUENCY, LORA_TX_POWER, LORA_SPREADING_FACTOR, LORA_SIGNAL_BANDWIDTH, LORA_CODING_RATE);
    SComm.onReceive(NetworkManager::handleReceived);
    eventBus.subscribe(NETWORK_DISCOVER, [this](Event e)
                       { this->networkDiscoverCallback(e); });
}

void NetworkManager::setAsServer()
{
    this->role = ROLE_SERVER;
}
// address of a master node
void NetworkManager::setAsClient(uint8_t addres)
{
    this->masterAddress = addres;
    this->role = ROLE_CLIENT;
}

void NetworkManager::sendToMaster(const String &message)
{
    Serial.print("[NETWORK] ");
    Serial.print(masterAddress);
    Serial.println(" Sending message to master");

    if (networkReady && role == ROLE_CLIENT)
    {
        sendTo(masterAddress, message);
    }
}

void NetworkManager::broadcast(const String &message)
{
    if (networkReady)
    {
        char msg[120];
        snprintf(msg, sizeof(msg), "%s", message.c_str());

        // Broadcast address is 0xFF
        SComm.sendUnreliable(0xFF, (uint8_t *)msg, strlen(msg));
    }
}

void NetworkManager::sendTo(uint8_t address, const String &message)
{
    if (networkReady)
    {
        char msg[120];
        snprintf(msg, sizeof(msg), "%s", message.c_str());

        SComm.sendReliable(address, (uint8_t *)msg, strlen(msg));
    }
}

void NetworkManager::handleReceived(const ReceivedPacket &packet)
{
    if (s_instance)
    {
        s_instance->onPacketReceived(packet); // Forward to instance method
    }
}

void NetworkManager::networkDiscoverCallback(Event e)
{
    if (s_instance)
    {
        s_instance->setAsClient(e.data1);
        #ifdef INFORMATION_NODE
        //do nothig only do thing if you are a playing node.
        //information node only listens
        #else
        String response = Protocol::buildDiscoverResponse(LORA_ADDRESS);
        Serial.print("respondindg with: ");
        Serial.println(response);
        s_instance->sendToMaster(response);
        #endif
    }
}

void NetworkManager::onPacketReceived(const ReceivedPacket &packet)
{
    Message msg;
    msg = Protocol::parse((const char *)packet.data, packet.dataLen);
    Serial.println("msg content");
    Serial.println(msg.type);
    Serial.println(msg.paramCount);
    Serial.println("msg content");
    bool result = confHandler.handleCommand(msg);
    if (!result)
    {
        Serial.println("Something went wrong at Configuration handler");
    }
}
