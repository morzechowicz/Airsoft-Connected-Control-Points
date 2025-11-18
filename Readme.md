## ESP32-Based Control Point Game

## What is this?

It's an ESP32-based project that utilizes LoRa for wireless communication.

---

## What does it do?

This project implements a simple "control point" game mode, similar to *Conquest* in Battlefield games. Two teams compete to control points on a map, earning score points for each control point they hold.

---

## How does it work?

- Each control point tracks which team currently owns it.
- Teams earn points over time for each control point they control.
- To capture a point, a team must hold down their team button (the large, colorful one) for a configurable amount of time (1–30 seconds).
- The game ends when either:
    - The timer runs out, **or**
    - One team reaches the target score (both are configurable).
- Final scores are displayed on all devices at the end of the game.

---

## How to configure

### old:
1. **Power on** all control points you want to use and place them in your chosen locations (not necessarily in that order).
2. **Configure settings** (TO DO: Add configuration instructions).
3. After confirming the configuration, the game timer will start.
4. When the timer reaches zero or a team wins, the final scores will be shown on all devices.

### new:
There is a new configuration app that works over BLE.
---

## Planned:

1. Memory for the main node. No game will be lost due to sudden power loss or unfair players.

*More detailed configuration instructions coming soon!*
