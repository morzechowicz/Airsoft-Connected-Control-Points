#pragma once

enum class GameState {
    Network,
    Config,
    CountDownSetup,
    CountDown,
    StartGame,
    Ongoing,
    Finished,
    WaitingForReset,
    Restore
};