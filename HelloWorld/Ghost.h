#pragma once

#include <memory>
#include "Utils.h"
#include "IGameBoard.h"
#include "FSM/GhostStateMachine.h"

enum class GlobalMode;

// Enumerations
enum class GhostType
{
	BLINKY,
	INKY,
	PINKY,
	CLYDE
};

// Ghost.h
// - represents a single enemy agent controlled by a finite-state-machine.
// - Keeps only data & plumbing. behavior implemented in FSM states (see FSM/*).
// - Uses IGameBoard for all external queries to avoid coupling to Game.
class Ghost
{
public:
	// lifecycle
	void Init(GhostType t, int startGX, int startGY, Play::Colour col);
	// Update returns true if this ghost collided with Pac-Man this tick.
	// Movement and FSM run here; resolution (eat vs player death) is handled by Game.
	bool Update(IGameBoard* board, int pacGX, int pacGY, float dt);
	void Draw() const;

	// event API
	void EnterFrightened(IGameBoard* board);
	void ExitFrightened(IGameBoard* board);
	void SetEaten(IGameBoard* board);
	void OnGlobalModeChange(IGameBoard* board, GlobalMode newMode);
	void ResetToSpawn();

	// FSM API

	// Build the FSM states and set the initial state.
	// OnEnter may receive nullptr during initialization; states must handle that.
	void InitStateMachine();
	void SetState(GhostState newState, IGameBoard* board = nullptr);
	GhostState GetState() const;

	// accessors used by states
	Play::Point2f GetPos() const { return pos; }
	int GetGX() const { return gx; }
	int GetGY() const { return gy; }

	// --- fields ---
	GhostType type{ GhostType::BLINKY };
	int gx = 0, gy = 0;
	int spawnGX = 0, spawnGY = 0;
	Play::Point2f pos{ 0,0 };
	Play::Point2f target{ 0,0 };
	Play::Point2f dir{ 0,0 };

	float speed = Cfg::BASE_GHOST_SPEED;
	float baseSpeed = Cfg::BASE_GHOST_SPEED;
	Play::Colour colour{ Play::cRed };
	Play::Colour baseColour{ Play::cRed };

private:
	std::unique_ptr<GhostStateMachine> m_fsm;
};
