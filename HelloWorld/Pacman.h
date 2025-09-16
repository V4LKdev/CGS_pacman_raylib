#pragma once

// Includes
#include "Utils.h"

// Forward Declarations
class Game;

// Class declaration
class Pacman
{
public:
	// Functions
	void Init(int startGX, int startGY);
	void HandleInput();
	void StepTowards(const Play::Point2f& tgt, float dt);
	void Update(Game* game, float dt);
	void Draw() const;

	// Variables
	int gx = 0, gy = 0;
	Play::Point2f pos{ 0,0 };
	Play::Point2f target{ 0,0 };
	Play::Point2f dir{ 0,0 };       // {-1,0,1}
	Play::Point2f queued{ 0,0 };    // input buffer
	float speed = 90.0f;
};

