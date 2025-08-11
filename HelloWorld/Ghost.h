#pragma once

// Includes
#include "Play.h"
#include "Utils.h"

// Forward Declarations
class Game;

// Enumerations
enum GhostType 
{ 
	BLINKY,
	INKY, 
	PINKY,
	CLYDE 
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

	// Variables
	GhostType type{ BLINKY };
	int gx = 0, gy = 0;
	Play::Point2f pos{ 0,0 };
	Play::Point2f target{ 0,0 };
	Play::Point2f dir{ 0,0 };
	float speed = 80.0f;
	Play::Colour colour{ Play::cRed };
};
