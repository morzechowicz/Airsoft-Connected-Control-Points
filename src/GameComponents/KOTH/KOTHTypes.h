// KOTHTypes.h
#ifndef KOTH_TYPES_H
#define KOTH_TYPES_H

#include <Arduino.h>
#include "Config.h"

// Team identification
enum class Team : uint8_t
{
    NONE = 0,
    YELLOW = 1,
    BLUE = 2
};
// ishould consolidate this at some point in the future
// yes future is now
// Node state
struct NodeState
{
    uint8_t nodeId;
    uint8_t type;
    Team controllingTeam;
    unsigned long capturedAt; // When it was captured  // guess i did fuck all with this parameter

    NodeState() : nodeId(0), type(UNDEFINED), controllingTeam(Team::NONE), capturedAt(0) {}
    NodeState(uint8_t id, uint8_t type, Team team, uint32_t time) : nodeId(id), type(type), controllingTeam(team), capturedAt(time) {}
    NodeState(uint8_t id, uint8_t type) : nodeId(id), type(type), controllingTeam(Team::NONE), capturedAt(0) {}
};

// Game scores
struct KOTHGameScore
{
    uint16_t yellowPoints;
    uint16_t bluePoints;

    KOTHGameScore() : yellowPoints(0), bluePoints(0) {}

    Team getWinner() const
    {
        if (yellowPoints > bluePoints)
            return Team::YELLOW;
        if (bluePoints > yellowPoints)
            return Team::BLUE;
        return Team::NONE; // Draw
    }
};

// KOTH Configuration
struct KOTHConfig
{
    uint16_t maxPoints;
    uint16_t gameDurationMinutes;
    uint16_t scoreIntervalMs;
    unsigned long captureTime;
    uint16_t respawnTime;

    // Node configuration
    NodeState NodeStates[MAX_NODES_COUNT]; // Actual node IDs in game
    uint8_t nodeCount;                     // How many nodes
    bool singleNodeMode;                   // Skip network if true

    KOTHConfig()
        : maxPoints(100),
          gameDurationMinutes(30),
          scoreIntervalMs(SCORING_INTERVAL_MS),
          nodeCount(0),
          singleNodeMode(false),
          captureTime(3000), // 3 seconds to capture
          respawnTime(5)     // 5 minutes respawn time

    {
        // Initialize node IDs to invalid
        for (int i = 0; i < MAX_NODES_COUNT; i++)
        {
            NodeStates[i] = {0xFF, 0xFF, Team::NONE, 0UL};
        }
    }

    // Helper to add a node
    void addNode(uint8_t nodeId, uint8_t type)
    {
        if (nodeCount < MAX_NODES_COUNT)
        {
            NodeStates[nodeCount++] = {nodeId, type};
        }
        singleNodeMode = (nodeCount == 1);
    }

    // Check if node is in game
    bool hasNode(uint8_t nodeId) const
    {
        for (uint8_t i = 0; i < nodeCount; i++)
        {
            if (NodeStates[i].nodeId == nodeId)
                return true;
        }
        return false;
    }

    uint8_t countNodesControlledBy(Team team)
    {
        uint8_t count = 0;
        for (uint8_t i = 0; i < MAX_NODES_COUNT; i++)
        {
            if (NodeStates[i].controllingTeam == team)
            {
                count++;
            }
        }
        return count;
    }

    NodeState* findNode(uint8_t nodeId)
    {
        for (uint8_t i = 0; i < MAX_NODES_COUNT; i++)
        {
            if (NodeStates[i].nodeId == nodeId)
            {
                return &NodeStates[i];
            }
        }
        return nullptr;
    }
};

#endif