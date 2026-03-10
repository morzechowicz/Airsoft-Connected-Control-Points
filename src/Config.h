//hardware configuration
#define BUTTON_BLUE_PIN 4
#define BUTTON_YELLOW_PIN 0
#define BUTTON_SELECT_PIN 2
#define BUTTON_ENTER_PIN 15

#define LED_BLUE_PIN 13
#define LED_YELLOW_PIN 12

#define BUZZER_PIN 14
#define BUZZER_GENERATOR false

#define SDA_PIN 21
#define SCL_PIN 22

// Pin definitions (adjust for your wiring!)
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_NSS 18
#define LORA_DIO0 26
#define LORA_RST 23
#define LORA_DIO1 32

// Radio parameters
// #define LORA_ADDRESS 0x02
#define LORA_FREQUENCY 433.0
#define LORA_TX_POWER 5
#define LORA_SPREADING_FACTOR 9
#define LORA_SIGNAL_BANDWIDTH 125.0
#define LORA_CODING_RATE 7

//ble configuration
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

//others
#define LOCALIZER_BEEP 3000UL;
#define SCORING_INTERVAL_MS 60000UL;
