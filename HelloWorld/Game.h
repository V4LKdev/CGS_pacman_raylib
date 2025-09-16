#pragma once

// Includes
#include <memory>

#include "Utils.h"
#include "Pacman.h"
#include "Ghost.h"
#include "IGameBoard.h"
#include "Modes.h"

// Forward Declarations
enum class GhostType;

// Game.h
// The central coordinator of the Pac-Man game.
// - Owns maze, Pac-Man, and ghosts
// - Implements IGameBoard for ghost AI queries
// - Manages global mode switching (Scatter/Chase), power-ups, and collisions
class Game : public IGameBoard
{
public:
	// Functions
	Game();

	static bool InBounds(int x, int y);
	bool IsWall(int x, int y) const override;

	Play::Point2f GetScatterTarget(GhostType type) const override;
	Play::Point2f GetPacDirection() const override;
	Play::Point2f GetGhostGrid(GhostType type) const override;
	Play::Point2f GetPacPosition() const override;
	GlobalMode GetGlobalMode() const override { return globalMode; }

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
	std::unique_ptr<Pacman> pac;
	std::vector<std::unique_ptr<Ghost>> ghosts;
	float powerUpTimer = 0.0f;
	bool powerUpPresent = false;
	GlobalMode globalMode = GlobalMode::Scatter;
	float modeTimer = Cfg::SCATTER_DURATION; // initial scatter time
	bool gameStarted = false;
};