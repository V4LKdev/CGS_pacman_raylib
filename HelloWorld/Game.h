#pragma once

// Includes
#include "Utils.h"

// Forward Declarations
class Ghost;
class Pacman;

// Class Declaration
class Game
{
public:
	// Functions
	Game();

	bool InBounds(int x, int y) const;
        bool IsWall(int x, int y) const;

        void BuildArena();
       void SpawnPowerUp();
       void ActivatePowerUp();

        void Init();
        void DrawMaze() const;

        void Update(float dt);

	void Draw() const;

        // Variables
        TileType maze[Cfg::GRID_HEIGHT][Cfg::GRID_WIDTH]{};
        Pacman* pac;
        std::vector<Ghost*> ghosts;
       float powerUpTimer = 0.0f;
       bool powerUpPresent = false;
};