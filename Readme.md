# SPAS-GS (SPAS Game System)

ESP32-based LoRa/BLE multiplayer game station framework for laser tag / capture-the-flag style games. Project is built with PlatformIO and targets `ttgo-lora32-v21` board.

## 🚀 Features

- ESP32 + LoRa (RadioLib) mesh game network
- BLE status/control interface
- LCD display (LiquidCrystal_I2C) and buzzer feedback
- Modular game modes (names are placeholders):
  - KOTH (King of the Hill)
  - FLAG (Capture the Flag)
- Master/client role management via `GameManager`
- Event-driven architecture (`EventBus`)
- Runtime configuration via `ConfigurationHandler`
- Optional information node mode (`INFORMATION_NODE`)

## 📦 Requirements

- Board: `ttgo-lora32-v21` (ESP32 + SX1276 LoRa)
- PlatformIO (recommended latest)
- Arduino framework

## 📁 Repository Structure

- `src/` core application code
  - `GameComponents/` game modes and base components
  - `Hardware/` button, LED, buzzer, lcd, manager
  - `Network/` message builder/parser and network manager
  - `BLE/` BLE setup and callbacks
  - `GameManager.{cpp,h}` orchestrates mode behavior
- `include/` project headers
- `lib/Logging/` custom logging utility
- `test/` unit tests (Protocol parser coverage)

## 🛠️ Build and Upload

1. Install PlatformIO and dependencies.
2. Set up `platformio.ini` profiles:
   - `env:device3` .. `env:device6` preset for Linux serial ports and addresses
   - `LORA_ADDRESS` must be unique for each node.
3. Build and upload:

\`\`\`bash
pio run -e device3 -t upload
\`\`\`

4. Monitor logs:

\`\`\`bash
pio device monitor -e device3
\`\`\`

## ⚙️ Compile-time Flags

- `-D LORA_ADDRESS=0xXX` (required unique node ID)
- `-D BIG_SCREEN` (4x20 LCD layout)
- `-D INFORMATION_NODE` (operation as information-only node)

## 🧠 Runtime Behavior

- `main.cpp` initializes subsystems
- `NetworkManager` discovery, sync, broadcast events
- `GameManager` handles config, score, game states
- `HardwareManager` poll buttons, update buzzer/LCD
- `EventBus` dispatches events between modules

## 🧪 Testing

- Unit tests in `/test` (PlatformIO + Unity)
- Run tests:

\`\`\`bash
pio test
\`\`\`

## 📌 Notes

- Logging includes serial + BLE outputs from `LogManager`.
- `BLE` uses `NimBLE-Arduino`.
- `LoRa` communication via `RadioLib`.
- Require additional apk for android to function properly