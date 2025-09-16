#pragma once

// Includes
#include "Utils.h"

enum class GlobalMode;
// Forward Declarations
class Game;

// Enumerations
enum class GhostType
{ 
	BLINKY,
	INKY, 
	PINKY,
	CLYDE 
};

enum class GhostState
{
	Idle,
	Scatter,
	Chase,
	Frightened,
	Eaten
};

// Class Declaration
class Ghost
{
public:
	// Functions
	void Init(GhostType t, int startGX, int startGY, Play::Colour col);
	Play::Point2f TickAI(const Game* game, int pacGX, int pacGY) const;
	void Update(Game* game, int pacGX, int pacGY, float dt);
	void Draw() const;

	void EnterFrightened();
	void ExitFrightened();
	void SetEaten();
	void OnGlobalModeChange(GlobalMode newMode);
	void ResetToSpawn();

	// Variables
	GhostType type{GhostType::BLINKY };
	int gx = 0, gy = 0;
	int spawnGX = 0, spawnGY = 0;
	Play::Point2f pos{ 0,0 };
	Play::Point2f target{ 0,0 };
	Play::Point2f dir{ 0,0 };
	// Current speed (may change when power-up active)
	float speed = 80.0f;
	// Stored default speed to restore after power-up expires
	float baseSpeed = 80.0f;
	// Current colour (may change when power-up active)
	Play::Colour colour{ Play::cRed };
	// Stored default colour to restore after power-up expires
	Play::Colour baseColour{ Play::cRed };

	// Statemachine
	GhostState state{ GhostState::Idle };
};
