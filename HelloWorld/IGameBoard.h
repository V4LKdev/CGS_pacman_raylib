#pragma once

#include "Utils.h"
#include "Modes.h"

enum class GhostType;

// IGameBoard.h
// Interface exposing read-only board queries for ghosts/FSM.
// Prevents circular dependency: FSM depends only on IGameBoard, not Game.
class IGameBoard {
public:
    virtual ~IGameBoard() = default;

    // World queries
    virtual bool IsWall(int x, int y) const = 0;

    // Targets and positions used by ghost AI
    virtual Play::Point2f GetScatterTarget(GhostType type) const = 0;
    virtual Play::Point2f GetPacDirection() const = 0;   // Pac-Man's grid direction
    virtual Play::Point2f GetGhostGrid(GhostType type) const = 0; // ghost grid coords by type
    virtual Play::Point2f GetPacPosition() const = 0;    // Pac-Man world position for collision

    // Global mode (Scatter/Chase)
    virtual GlobalMode GetGlobalMode() const = 0;
};
