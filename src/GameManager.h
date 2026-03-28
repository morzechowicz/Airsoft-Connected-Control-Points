#ifndef GameManager_h
#define GameManager_h

#include "GameComponents/KOTH/KOTHTypes.h"
#include "GameComponents/KOTH/KOTHServer.h"
#include "GameComponents/KOTH/KOTHClient.h"
#include "GameComponents/FLAG/FLAGServer.h"
#include "GameComponents/FLAG/FLAGClient.h"
#include "GameComponents/InformationModeComp.h"
#include "Config.h"
#include "EventBus.h"
#include "Hardware/HardwareManager.h"
#include "Network/NetworkManager.h"
#include "ConfigurationHandler.h"
#include "../lib/Logging/LogManager.h"

class GameManager
{
private:
    EventBus *eventBus;
    HardwareManager *hardwareManager;
    NetworkManager *networkManager;

    KOTHConfig kothConfig;

    KOTHServer *kothServer = nullptr;
    KOTHClient *kothClient = nullptr;

    FLAGConfig flagConfig;

    FLAGServer *flagServer = nullptr;
    FLAGClient *flagClient = nullptr;

    InformationModeComp *infoNode = nullptr;

    bool isMaster = false;

    EventType selectedConfig = CONF;
    TaskHandle_t countdownHandler = nullptr;

public:
    GameManager(EventBus *eb, HardwareManager *hw, NetworkManager *net)
        : eventBus(eb), hardwareManager(hw), networkManager(net) {};
    ~GameManager();

    static GameManager* instance;

    void onNewNode(Event e);
    void onGameStarted(Event e);
    void onConfigKothFromMaster(Event e);
    void onConfKoth(Event e);
    void onConfigureFlag(Event e);
    void onDiscoverRequest(Event e);
    void onDiscovered(Event e);
    void update();

    void countdownTask(int time);
    void startCountdownTask(int countdown);
};

#endif // GameManager_h