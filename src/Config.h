#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>

//define ini values
//NODE TYPE
#define BLEToLoRa 1
#define INFORMATION 2
#define CAPTURE_POINT 3

//CHIP TYPE
#define HELTECSX1262 1
#define RA_02_SX1278 2

//SCREEN TYPE
#define SMOLL_SCREEN 1
#define CHONKY_SCREEN 2
#define NONE_SCREEN 0

//BUZZER GENERATOR
#define BUZZER_ON 1
#define BUZZER_OFF 0

//Node address

#ifdef LORA_ADDRESS

#else
#error "LORA_ADDRESS is not defined, please define it in the build configuration"
#endif

//hardware configuration
#define BUTTON_BLUE_PIN 4
#define BUTTON_YELLOW_PIN 0
#define BUTTON_SELECT_PIN 2
#define BUTTON_ENTER_PIN 15

#define LED_BLUE_PIN 13
#define LED_YELLOW_PIN 12

#define BUZZER_PIN 14



// Pin definitions (adjust for your wiring!)
#if SX_CHIP_TYPE == HELTECSX1262
#define LORA_SCK 9
#define LORA_MISO 11
#define LORA_MOSI 10
#define LORA_NSS 8
#define LORA_BUSY 13 
#define LORA_RST 12
#define LORA_DIO1 14

#define SDA_PIN 19
#define SCL_PIN 20
#elif SX_CHIP_TYPE == RA_02_SX1278
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_NSS 18
#define LORA_DIO0 26
#define LORA_RST 23
#define LORA_DIO1 32

#define SDA_PIN 21
#define SCL_PIN 22
#else
#error "Unknown SX_CHIP_TYPE, please define it as HELTECSX1262 or RA_02_SX1278"
#endif

// Radio parameters
// #define LORA_ADDRESS 0x02
#define LORA_FREQUENCY 433.200
#define LORA_TX_POWER 8
#define LORA_SPREADING_FACTOR 7
#define LORA_SIGNAL_BANDWIDTH 250.0
#define LORA_CODING_RATE 5

//ble configuration
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

//others
#define LOCALIZER_BEEP_LAST_MINUTE 1000UL
#define LOCALIZER_BEEP_ONE_FOURTH 2000UL
#define LOCALIZER_BEEP_ONE_HALF 4000UL
#define LOCALIZER_BEEP_THREE_FOURTH 6000UL
#define LOCALIZER_BEEP_FULL 9000UL
#define SCORING_INTERVAL_MS 60000UL

#endif // CONFIG_H