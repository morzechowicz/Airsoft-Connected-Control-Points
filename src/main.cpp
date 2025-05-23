#include <Arduino.h>
#include <Team.h>
#include <Teams.h>
#include <LoRaManager.h>
#include <RadioLib.h>
#include <ButtonManager.h>
#include <GameState.h>
#include <DisplayOled.h>
#include <Config.h>
#include <ControlPoint.h>
#include <Clocker.h>
#include <LoRaHandler.h>
#include "LoRaMsg.h"


GameState gameState;
ButtonManager buttonManager;
ControlPoint controlPoint(2);
DisplayOled displayOLED;
Config config(15, 15, 15, 15);
Clocker pointClock;
Clocker secondCLock;
Clocker gameClock;
// LoRaManager lora;
LoRaMsgHandler msgHandler(config, controlPoint, gameState);
SX1278 radio = new Module(18,26,14,44);
LoRaCom loraCom(radio);
LoRaHandler lrradio(loraCom,msgHandler);
LoRaMsg loramsg(loraCom, config, controlPoint);

// TO DO:
//  displaying info on lcd and oled logic
//  lora logic mainly keeping points in main node and getting team changes from other node

TeamId winner = TeamId::None;
int configState = 0;
bool MainNode = false;

bool isGameOver()
{
    // time overflow
    if (gameClock.getElapsedTimeInMinutes() > config.getDurration())
    {
        Serial.println("TIMES UP");
        return true;
    }
    // winning by points
    if (controlPoint.pointsTargetReached(config.getPointsTarget()))
    {
        Serial.println("POINTS REACHED");
        return true;
    }
    return false;
}

static Clocker captureClock;
static Clocker graceClock;
static TeamId capturingTeam = TeamId::None;
static bool gracePeriodActive = false;

void CapturePoint()
{
    if (buttonManager.blueButton.getState() == LOW)
    {
        if (capturingTeam != TeamId::Blufor && controlPoint.getControllingTeam() != TeamId::Blufor)
        {
            capturingTeam = TeamId::Blufor;
            captureClock.start();
            captureClock.reset();
            Serial.println("Team Blue started capturing!");
        }
        if ((captureClock.getElapsedTimeInSeconds() >= config.getCaptureTime() / 2) && controlPoint.getControllingTeam() != TeamId::None && controlPoint.getControllingTeam() != TeamId::Blufor)
        {
            controlPoint.setControllingTeam(TeamId::None);
            Serial.println("Point neutralized!");
        }
        if (captureClock.getElapsedTimeInSeconds() >= config.getCaptureTime() && controlPoint.getControllingTeam() != TeamId::Blufor)
        {
            controlPoint.setControllingTeam(TeamId::Blufor);
            Serial.println("Team Blue fully captured the point!");
        }
        gracePeriodActive = false;
        float percentage = static_cast<float>(captureClock.getElapsedTimeInSeconds()) / static_cast<float>(config.getCaptureTime());
        if (percentage > 1)
        {
            percentage = 1;
        }
        displayOLED.displayCapturing(capturingTeam, percentage);
    }
    else if (buttonManager.yellowButton.getState() == LOW)
    {
        if (capturingTeam != TeamId::YellowFor && controlPoint.getControllingTeam() != TeamId::YellowFor)
        {
            capturingTeam = TeamId::YellowFor;
            captureClock.start();
            captureClock.reset();
            Serial.println("Team Yellow started capturing!");
        }
        if ((captureClock.getElapsedTimeInSeconds() >= config.getCaptureTime() / 2) && controlPoint.getControllingTeam() != TeamId::None && controlPoint.getControllingTeam() != TeamId::YellowFor)
        {
            controlPoint.setControllingTeam(TeamId::None);
            Serial.println("Point neutralized!");
        }
        if (captureClock.getElapsedTimeInSeconds() >= config.getCaptureTime() && controlPoint.getControllingTeam() != TeamId::YellowFor )
        {
            controlPoint.setControllingTeam(TeamId::YellowFor);
            Serial.println("Team Yellow fully captured the point!");
        }
        gracePeriodActive = false;
        float percentage = static_cast<float>(captureClock.getElapsedTimeInSeconds()) / static_cast<float>(config.getCaptureTime());
        if (percentage > 1)
        {
            percentage = 1;
        }
        displayOLED.displayCapturing(capturingTeam, percentage);
    }
    else
    {
        if (!gracePeriodActive)
        {
            gracePeriodActive = true;
            graceClock.reset();
            graceClock.start();
        }
        if (graceClock.getElapsedTime() > 500)
        {
            capturingTeam = TeamId::None;
            captureClock.stop();
            captureClock.reset();
            graceClock.stop();
            graceClock.reset();
            gracePeriodActive = false;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting up...");
    displayOLED.begin();
    gameState = GameState::Config;
    controlPoint.addTeam(TeamId::Blufor);
    controlPoint.addTeam(TeamId::YellowFor);
    buttonManager.begin();
    lrradio.begin();

    // lora.begin();
    Serial.println("Ready");
}

void loop()
{
    lrradio.loop();
    // lora.recivingLoop();
    switch (gameState)
    {
    case GameState::Config:
        config.handleButtonPresses(buttonManager, configState);
        if (buttonManager.startButton.isPressed())
        {
            // lora.sendNewConfig(config);
            gameState = GameState::CountDown;
            secondCLock.start();
        }
        displayOLED.displaySettings(config, configState);
        break;
    case GameState::CountDown:
        if (secondCLock.getElapsedTime() > 1000) // for the time being this stays like this i cant be bothered to add another setting rihgt now also nobody cares
        {
            config.setCountdown(config.getCountdown() - 1);
            Serial.println("Countdown: ");
            Serial.println(config.getCountdown());
            // secondCLock.getElapsedTime(); // add this to countdown display later TODO
            secondCLock.reset();
        }
        if (config.getCountdown() <= 0)
        {
            secondCLock.stop();
            secondCLock.reset();
            gameState = GameState::Ongoing;
            pointClock.start();
            gameClock.start();
        }
        displayOLED.displayCountdown(config.getCountdown());
        break;
    case GameState::Ongoing:
        if (pointClock.getElapsedTime() >= 60000) // like the last one :/ will do later
        {
            Serial.println("Time elapsed: ");
            Serial.println(gameClock.getElapsedTimeInMinutes());
            controlPoint.increamentScore(1);
            if (isGameOver())
            {
                gameState = GameState::Finished;
                gameClock.stop();
                gameClock.reset();

                winner = controlPoint.whoWon();
                Serial.println("GAME OVER");
                if (winner == TeamId::Blufor)
                {
                    Serial.print("BLUFOR WON");
                }
                if (winner == TeamId::YellowFor)
                {
                    Serial.print("YELLOWFOR WON");
                }
                if (winner == TeamId::Draw)
                {
                    Serial.print("DRAW");
                }
                if (winner == TeamId::None)
                {
                    Serial.print("a NONE in a WINNER if? how queer! Ive never seen such a thing ");
                    Serial.println("I guess we make an none now");
                    Serial.println("for real i have no idea what to put here");
                }
                pointClock.stop();
            }
            pointClock.reset();
        }
        // Capture Logic
        CapturePoint();
        if (buttonManager.yellowButton.getState() == HIGH && buttonManager.blueButton.getState() == HIGH)
        {
            displayOLED.displayGame(controlPoint, config.getDurration() - gameClock.getElapsedTimeInMinutes());
        }
        break;
    case GameState::Finished:
        // here we just display winner until idk button press or what ever
        if (buttonManager.changeButton.isPressed())
        {
            gameState = GameState::Config;
        }
        displayOLED.displayFinished(winner, controlPoint);
        break;
    default:
        // maybe just set state to config? not like there are any other ways default could happen
        gameState = GameState::Config;
        break;
    }
    buttonManager.update();
}
// #include <Arduino.h>
// #include <RadioLib.h>
// #include <ButtonManager.h>

// ButtonManager buttonManager;
// SX1278 radio = new Module(18, 26, 14, 33);

// volatile bool receivedFlag = false;
// int transmissionState = RADIOLIB_ERR_NONE;

// // flag to indicate that a packet was sent
// volatile bool transmittedFlag = false;
// void setFlagR(void)
// {
//     // we got a packet, set the flag
//     receivedFlag = true;
// }

// void setFlagS(void)
// {
//     // we sent a packet, set the flag
//     transmittedFlag = true;
// }
// void setup()
// {
//     Serial.begin(115200);
//     Serial.println("Starting up...");
//     buttonManager.begin();
//     radio.begin();

//     // initialize SX1278 with default settings
//     Serial.print(F("[SX1278] Initializing ... "));
//     int state = radio.begin();
//     if (state == RADIOLIB_ERR_NONE)
//     {
//         Serial.println(F("success!"));
//     }
//     else
//     {
//         Serial.print(F("failed, code "));
//         Serial.println(state);
//         while (true)
//         {
//             delay(10);
//         }
//     }

//     radio.setDio0Action(setFlagR,RISING);
//     // start listening for LoRa packets
//     Serial.print(F("[SX1278] Starting to listen ... "));
//     state = radio.startReceive();
//     if (state == RADIOLIB_ERR_NONE)
//     {
//         Serial.println(F("success!"));
//     }
//     else
//     {
//         Serial.print(F("failed, code "));
//         Serial.println(state);
//         while (true)
//         {
//             delay(10);
//         }
//     }
//     Serial.println("Ready");
// }
// int count = 0;
// void loop()
// {
//     buttonManager.update();
//     if (buttonManager.changeButton.isPressed())
//     {
//         String str = "Hello World! #" + String(count++);
//         transmissionState = radio.transmit(str);

//         Serial.println("Change button pressed");

//         if (transmissionState == RADIOLIB_ERR_NONE)
//         {
//             // packet was successfully sent
//             Serial.println(F("transmission finished!"));

//             // NOTE: when using interrupt-driven transmit method,
//             //       it is not possible to automatically measure
//             //       transmission data rate using getDataRate()
//         }
//         else
//         {
//             Serial.print(F("failed, code "));
//             Serial.println(transmissionState);
//         }
//     }
//     if (receivedFlag)
//     {
//         // reset flag
//         receivedFlag = false;

//         // you can read received data as an Arduino String
//         String str;
//         int state = radio.readData(str);

//         // you can also read received data as byte array
//         /*
//           byte byteArr[8];
//           int numBytes = radio.getPacketLength();
//           int state = radio.readData(byteArr, numBytes);
//         */

//         if (state == RADIOLIB_ERR_NONE)
//         {
//             // packet was successfully received
//             Serial.println(F("[SX1278] Received packet!"));

//             // print data of the packet
//             Serial.print(F("[SX1278] Data:\t\t"));
//             Serial.println(str);

//             // print RSSI (Received Signal Strength Indicator)
//             Serial.print(F("[SX1278] RSSI:\t\t"));
//             Serial.print(radio.getRSSI());
//             Serial.println(F(" dBm"));

//             // print SNR (Signal-to-Noise Ratio)
//             Serial.print(F("[SX1278] SNR:\t\t"));
//             Serial.print(radio.getSNR());
//             Serial.println(F(" dB"));

//             // print frequency error
//             Serial.print(F("[SX1278] Frequency error:\t"));
//             Serial.print(radio.getFrequencyError());
//             Serial.println(F(" Hz"));
//         }
//         else if (state == RADIOLIB_ERR_CRC_MISMATCH)
//         {
//             // packet was received, but is malformed
//             Serial.println(F("[SX1278] CRC error!"));
//         }
//         else
//         {
//             // some other error occurred
//             Serial.print(F("[SX1278] Failed, code "));
//             Serial.println(state);
//         }
//     }
// }