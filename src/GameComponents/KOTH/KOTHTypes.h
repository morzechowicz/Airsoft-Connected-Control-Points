// KOTHTypes.h
#ifndef KOTH_TYPES_H
#define KOTH_TYPES_H

#include <Arduino.h>

// Team identification
enum class Team : uint8_t {
    NONE = 0,
    YELLOW = 1,
    BLUE = 2
};
// ishould consolidate this at some point in the future
// Node state
struct NodeState {
    uint8_t nodeId;
    Team controllingTeam;
    unsigned long capturedAt;  // When it was captured
    
    NodeState() : nodeId(0), controllingTeam(Team::NONE), capturedAt(0) {}
};

// Nodes id and other info
struct NodeInit {
    uint8_t Id;
    uint8_t type;
};

// Game scores
struct KOTHGameScore {
    uint16_t yellowPoints;
    uint16_t bluePoints;
    
    KOTHGameScore() : yellowPoints(0), bluePoints(0) {}
    
    Team getWinner() const {
        if (yellowPoints > bluePoints) return Team::YELLOW;
        if (bluePoints > yellowPoints) return Team::BLUE;
        return Team::NONE;  // Draw
    }
};

// KOTH Configuration
struct KOTHConfig {
    uint16_t maxPoints;
    uint16_t gameDurationMinutes;
    uint16_t scoreIntervalMs;
    unsigned long captureTime;
    uint16_t respawnTime;
    
    // Node configuration
    NodeInit nodeIds[10];     // Actual node IDs in game
    uint8_t nodeCount;       // How many nodes
    bool singleNodeMode;     // Skip network if true
    
    KOTHConfig() 
        : maxPoints(100),
          gameDurationMinutes(30),
          scoreIntervalMs(30000),  // 30 seconds
          nodeCount(0),
          singleNodeMode(false),
          captureTime(3000),  // 3 seconds to capture
          respawnTime(5)   // 5 minutes respawn time
    
        {
        // Initialize node IDs to invalid
        for (int i = 0; i < 10; i++) {
            nodeIds[i] = {0xFF, 0xFF};
        }
    }
    
    // Helper to add a node
    void addNode(uint8_t nodeId, uint8_t type) {
        if (nodeCount < 10) {
            nodeIds[nodeCount++] = {nodeId, type};
        }
        singleNodeMode = (nodeCount == 1);
    }
    
    // Check if node is in game
    bool hasNode(uint8_t nodeId) const {
        for (uint8_t i = 0; i < nodeCount; i++) {
            if (nodeIds[i].Id == nodeId) return true;
        }
        return false;
    }
};

#endif