// #include "InformationModeComp.h"

// InformationModeComp::InformationModeComp(EventBus *eventBus, HardwareManager *hardware, NetworkManager *network) :
//     eventBus(eventBus),
//     hardware(hardware),
//     network(network)
// {

// }

// void InformationModeComp::start()
// {
//     Serial.println("Starting Information Mode Component");
//         // Subscribe to game events
//     eventBus->subscribe(GAME_OVER, [this](Event e)
//                         {
//         handleGameOver(lastKnownScore.getWinner());
//         gameActive = false;
//         updateDisplay();
//         return;
//     });

//     // Subscribe to score updates
//     eventBus->subscribe(KOTH_SCORE_UPDATE, [this](Event e)
//                         {
//         lastKnownScore.yellowPoints = e.data1;
//         lastKnownScore.bluePoints = e.data2;
//         updateDisplay(); });
//     updateDisplay();
// }
