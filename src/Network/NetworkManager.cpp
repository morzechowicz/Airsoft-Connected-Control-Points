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
    LOG_INFO("NETWORK", "Sending message to master (0x%02X)", masterAddress);

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

        SComm.sendReliable(address, (uint8_t *)msg, strlen(msg),nullptr,0);
    }
}

bool NetworkManager::sendToAll(const std::vector<uint8_t>& addresses, const String& message)
{
    if (!networkReady) return false;

    EventGroupHandle_t ackEvents = xEventGroupCreate();
    EventBits_t expectedBits = 0;

    char msg[120];
    snprintf(msg, sizeof(msg), "%s", message.c_str());

    // Fire all sends without waiting
    for (int i = 0; i < addresses.size(); i++) {
        if (addresses[i] == myAddress) continue;

        EventBits_t bit = (1 << i);
        expectedBits |= bit;

        SComm.sendReliable(
            addresses[i],
            (uint8_t*)msg, strlen(msg),
            ackEvents, bit           // <-- new params
        );
    }

    // Block until all ACKed or timeout
    EventBits_t result = xEventGroupWaitBits(
        ackEvents,
        expectedBits,
        pdTRUE,              // clear bits on exit
        pdTRUE,              // wait for ALL bits
        pdMS_TO_TICKS(5000)
    );

    vEventGroupDelete(ackEvents);

    if ((result & expectedBits) == expectedBits) {
        LOG_INFO("NETWORK", "All nodes confirmed");
        return true;
    }

    // Log exactly who failed
    for (int i = 0; i < addresses.size(); i++) {
        if (!(result & (1 << i))) {
            LOG_ERROR("NETWORK", "Node 0x%02X did not ACK", addresses[i]);
        }
    }
    return false;
}

void NetworkManager::sendToUnreliable(uint8_t address, const String &message)
{
    if (networkReady)
    {
        char msg[120];
        snprintf(msg, sizeof(msg), "%s", message.c_str());

        SComm.sendUnreliable(address, (uint8_t *)msg, strlen(msg));
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
        String response = Protocol::buildDiscoverResponse(LORA_ADDRESS);
        LOG_INFO("NETWORK", "Responding with: %s", response.c_str());
        s_instance->sendToMaster(response);
    }
}

void NetworkManager::broadcastReset() {
    String msg = Protocol::buildPowerResetMsg();
    broadcast(msg);
}

void NetworkManager::onPacketReceived(const ReceivedPacket &packet)
{
    Message msg;
    msg = Protocol::parse((const char *)packet.data, packet.dataLen);
    LOG_INFO("NETWORK", "Received message: %s", (const char *)packet.data);
    bool result = confHandler.handleCommand(msg);
    if (!result)
    {
        LOG_ERROR("NETWORK", "Something went wrong at Configuration handler");
    }
}
