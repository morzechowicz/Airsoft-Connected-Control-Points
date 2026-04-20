# SPAS-GS (SPAS Game System)

ESP32-based system for outdoor games.

Currently supports "capture the point" style gameplay with automatic LoRa communication between nodes and simple configuration via Android app *([linked here](https://github.com/morzechowicz/ACCP-android-app))*.

## Features

- ESP32 + LoRa (RadioLib) point-to-point network
- BLE status/control interface
- LCD display (LiquidCrystal_I2C) and buzzer feedback
- Modular architecture supporting multiple game modes *(currently one game mode; more coming soon)*
- Server-client architecture

## Node Roles

**Capture Point** — Core gameplay node. Acts as the contested point players fight over. Can serve as both server and client simultaneously. Required for all games.

**Information** — Displays current game state and score information.

**Repeater** — Extends network range by retransmitting all received messages. Capture nodes handle deduplication of repeated messages.

**BLE-to-LoRa** — Carried by the game organizer. Translates BLE messages to LoRa and forwards them to targeted nodes, enabling remote configuration of capture nodes from a distance.

## example usage:

Lets say you have 2 control points(CP), one repeater and 2 information nodes(IN).

you need to place them is such a way that point you designate as server will see all nodes or in a way that it sees some points and repeater and repeater sees remaining points. repeater will send server commands to the points server does not see.

Now you need to make sure that server knows about other cp and then you can start game *([see app instructions - not ready](https://github.com/morzechowicz/ACCP-android-app))*.

game will start after countdown and end on achiving point limit or time limit or on command.

## Supported Boards

For capture point and information nodes:
- ESP32 WROOM + RA-02 SX1278 433MHz LoRa module
- TTGO LoRa32

For repeater and BLE-to-LoRa nodes:
- Heltec Wireless Stick v3

## Required Materials — Single Control Node

- ESP32
- 4×16 or 4×20 LCD screen with I2C
- 2 buttons with LEDs
- Buck converter to 5V
- MOSFET IRLZ44N
- Buzzer with built-in oscillator
- Cables, resistors, etc.

Depending on board:
- RA-02 SX1278 433MHz LoRa module
- 433 MHz antenna

## Repository Structure

```
src/
├── GameComponents/     game modes and base components
├── Hardware/           button, LED, buzzer, LCD manager
├── Network/            message builder/parser, network manager
├── BLE/                BLE setup and callbacks
└── GameManager.{cpp,h} orchestrates mode behavior

lib/
└── Logging/            custom logging utility
```

## Build

Built with PlatformIO. See `platformio.ini` for environment configuration.

Everything can also be changed in configuration file Config.h. You can also find there pins outputs.

## Compile-time Flags

Example configuration for a standard capture point node:

```ini
[env:controlpoint]
extends = env
board = ttgo-lora32-v21
upload_port = /dev/ttyACM1
build_flags =
    -D LORA_ADDRESS=0x03
    -D NODE_TYPE=CAPTURE_POINT
    -D SX_CHIP_TYPE=RA_02_SX1278
    -D SCREEN_TYPE=LCD_SMOLL_SCREEN
    -D BUZZER_GENERATOR=BUZZER_OFF
```

## Runtime Behavior

- `main.cpp` — initializes all subsystems
- `NetworkManager` — handles discovery, sync, and broadcast events
- `GameManager` — manages config, score, and game states
- `HardwareManager` — polls buttons, updates buzzer and LCD
- `EventBus` — dispatches events between modules

## Notes

- If something is in the code but not documented here, it probably doesn't work yet.
- This project is in early development — everything is subject to change.
- Use this code at your own risk.