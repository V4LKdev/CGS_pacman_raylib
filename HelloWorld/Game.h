#pragma once

// Includes
#include "Utils.h"

// Forward Declarations
class Ghost;
class Pacman;
enum class GhostType;

enum class GlobalMode { Scatter, Chase };

// Class Declaration
class Game
{
public:
	// Functions
	Game();

	static bool InBounds(int x, int y);
	bool IsWall(int x, int y) const;

	Play::Point2f GetScatterTarget(GhostType type) const;
	Play::Point2f GetPacDirection() const;
	Play::Point2f GetGhostGrid(GhostType type) const;

	void BuildArena();
	void SpawnPowerUp();
	void ActivatePowerUp();

	void Init();
	void DrawMaze() const;

	void Update(float dt);

	void Draw() const;

	static bool CheckCollision(const Play::Point2f& a, const Play::Point2f& b, const float radius = Cfg::TILE_SIZE * 0.5f)
	{
		float dx = a.x - b.x;
		float dy = a.y - b.y;
		return (dx*dx + dy*dy) <= (radius*radius);
	}

	// Variables
	TileType maze[Cfg::GRID_HEIGHT][Cfg::GRID_WIDTH]{};
	Pacman* pac;
	std::vector<Ghost*> ghosts;
	float powerUpTimer = 0.0f;
	bool powerUpPresent = false;
	GlobalMode globalMode = GlobalMode::Scatter;
	float modeTimer = 7.0f; // initial scatter time
	bool gameStarted = false;
};