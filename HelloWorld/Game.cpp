// This file's header
#include "Game.h"

// Other includes
#include "Ghost.h"
#include "Pacman.h"
#include <random>

Game::Game()
{
	pac = std::make_unique<Pacman>();

	ghosts.resize(4);
	for (int i = 0; i <= 3; ++i)
	{
		ghosts[i] = std::make_unique<Ghost>();
	}
}

bool Game::InBounds(int x, int y)
{
	return x >= 0 && y >= 0 && x < Cfg::GRID_WIDTH && y < Cfg::GRID_HEIGHT;
}

bool Game::IsWall(int x, int y) const
{
	if (!InBounds(x, y))
	{
		return true;
	}

	return maze[y][x] == TileType::WALL;
}

Play::Point2f Game::GetScatterTarget(GhostType type) const
{
	switch (type)
	{
	case GhostType::BLINKY: return { Cfg::GRID_WIDTH - 2, 1 };                 // top-right
	case GhostType::PINKY:  return { 1, 1 };                                   // top-left
	case GhostType::INKY:   return { Cfg::GRID_WIDTH - 2, Cfg::GRID_HEIGHT-2 };// bottom-right
	case GhostType::CLYDE:  return { 1, Cfg::GRID_HEIGHT - 2 };                // bottom-left
	}
	return {0,0};
}

Play::Point2f Game::GetPacDirection() const
{
	return pac->dir;
}

Play::Point2f Game::GetPacPosition() const
{
	return pac ? pac->pos : Play::Point2f{0,0};
}

Play::Point2f Game::GetGhostGrid(GhostType type) const
{
	for (const std::unique_ptr<Ghost>& g : ghosts)
	{
		if (g->type == type)
		{
			return { (float)g->gx, (float)g->gy };
		}
	}
	return {0,0}; // fallback
}

void Game::BuildArena()
{
	for (int y = 0; y < Cfg::GRID_HEIGHT; ++y)
	{
		for (int x = 0; x < Cfg::GRID_WIDTH; ++x)
		{
			const bool border = (x == 0 || y == 0 || x == Cfg::GRID_WIDTH - 1 || y == Cfg::GRID_HEIGHT - 1);
			maze[y][x] = border ? TileType::WALL : TileType::PELLET;
		}
	}
}

void Game::SpawnPowerUp()
{
	// Gather all tiles that currently contain a pellet
	std::vector<Play::Point2f> candidates;
	for (int y = 0; y < Cfg::GRID_HEIGHT; ++y)
	{
		for (int x = 0; x < Cfg::GRID_WIDTH; ++x)
		{
			if (maze[y][x] == TileType::PELLET)
			{
				candidates.emplace_back( x, y );
			}
		}
	}

	if (!candidates.empty())
	{
		static std::random_device rd;
		static std::mt19937 rand(rd());
		std::uniform_int_distribution<int> dist(0, static_cast<int>(candidates.size()) - 1);

		const int idx = dist(rand);
		const Play::Point2f choice = candidates[idx];
		maze[static_cast<int>(choice.y)][static_cast<int>(choice.x)] = TileType::POWERUP;
		powerUpPresent = true;
	}
}

void Game::ActivatePowerUp()
{
	powerUpTimer = Cfg::POWERUP_DURATION;
	powerUpPresent = false;
	for (const std::unique_ptr<Ghost>& g : ghosts)
	{
		g->EnterFrightened(this);
	}
}

void Game::Init()
{
	BuildArena();

	// Player start
	pac->Init(13 - 4, 23);

	// Ghosts
	constexpr int cx = Cfg::GRID_WIDTH / 2;
	constexpr int cy = Cfg::GRID_HEIGHT / 2;
	ghosts[0]->Init(GhostType::BLINKY, cx - 2, cy, Play::cRed);
	ghosts[1]->Init(GhostType::INKY, cx, cy, Play::cCyan);
	ghosts[2]->Init(GhostType::PINKY, cx + 2, cy, Play::cMagenta);
	ghosts[3]->Init(GhostType::CLYDE, cx, cy + 2, Play::cOrange);

	SpawnPowerUp();
}

void Game::DrawMaze() const
{
	for (int y = 0; y < Cfg::GRID_HEIGHT; ++y)
	{
		for (int x = 0; x < Cfg::GRID_WIDTH; ++x)
		{
			int px = x * Cfg::TILE_SIZE, py = y * Cfg::TILE_SIZE;
			if (maze[y][x] == TileType::WALL)
			{
				Play::DrawRect({ px, py }, { px + Cfg::TILE_SIZE - 1, py + Cfg::TILE_SIZE - 1 }, Play::cBlue, true);
			}				
			else if (maze[y][x] == TileType::PELLET)
			{
				Play::DrawCircle({ px + Cfg::TILE_SIZE / 2, py + Cfg::TILE_SIZE / 2 }, Cfg::PELLET_RADIUS, Play::cWhite);
			}
			else if (maze[y][x] == TileType::POWERUP)
			{
				Play::DrawCircle({ px + Cfg::TILE_SIZE / 2, py + Cfg::TILE_SIZE / 2 }, Cfg::POWERUP_RADIUS, Play::cYellow);
			}
		}
	}

}

void Game::Update(float dt)
{
	// Main game loop step:
	// - Update timers (mode & power-ups)
	// - Update Pac-Man and ghosts
	// - Resolve collisions
	// - Handle idle → scatter/chase transition once player moves


	if (powerUpTimer <= 0.0f)
	{
		modeTimer -= dt;
		if (modeTimer <= 0.0f)
		{
			if (globalMode == GlobalMode::Scatter)
			{
				globalMode = GlobalMode::Chase;
				modeTimer = Cfg::CHASE_DURATION;
			} else {
				globalMode = GlobalMode::Scatter;
				modeTimer = Cfg::SCATTER_DURATION;
			}
			for (const std::unique_ptr<Ghost>& g : ghosts) {
				g->OnGlobalModeChange(this, globalMode);
			}
		}
	}

	pac->HandleInput();
	pac->Update(this, dt);

	if (powerUpTimer > 0.0f)
	{
		powerUpTimer -= dt;
		if (powerUpTimer <= 0.0f)
		{
			powerUpTimer = 0.0f;
			for (const std::unique_ptr<Ghost>& g : ghosts)
			{
				g->ExitFrightened(this);
			}				
		}
	}
	else if (!powerUpPresent)
	{
		// Power-up expired and none present on the board
		SpawnPowerUp();
	}

	for (const std::unique_ptr<Ghost>& g : ghosts)
	{
		bool collidedWithPac = g->Update(this, pac->gx, pac->gy, dt);

		// Handle collision with Pacman if detected
		if (collidedWithPac)
		{
			if (g->GetState() == GhostState::Frightened)
			{
				g->SetEaten(this);
			}
			else if (g->GetState() != GhostState::Eaten)
			{
				pac->ResetToSpawn();
				for (const std::unique_ptr<Ghost>& ghost : ghosts)
					ghost->ResetToSpawn();
				gameStarted = false;
			}
		}
	}


	// Changes ghosts from idle to scatter when Pacman starts moving
	if (!gameStarted && pac->startedMoving)
	{
		gameStarted = true;
		for (const std::unique_ptr<Ghost>& g : ghosts)
		{
			if (g->GetState() == GhostState::Idle)
			{
				g->SetState(globalMode == GlobalMode::Scatter ? GhostState::Scatter : GhostState::Chase, this);
			}
		}
	}
}

void Game::Draw() const
{
	DrawMaze();
	pac->Draw();

	for (const std::unique_ptr<Ghost>& g : ghosts)
	{
		g->Draw();
	}

	// Win text
	bool pelletsLeft = false;
	for (int y = 0; y < Cfg::GRID_HEIGHT && !pelletsLeft; ++y)
	{
		for (int x = 0; x < Cfg::GRID_WIDTH; ++x)
		{
			if (maze[y][x] == TileType::PELLET) pelletsLeft = true;
		}
	}

	if (!gameStarted)
	{
		Play::DrawDebugText({ Cfg::DISPLAY_W / 2, Cfg::DISPLAY_H / 2 }, "PRESS ARROW KEY TO START", 20);
	}
	else if (!pelletsLeft)
	{
		Play::DrawDebugText({ Cfg::DISPLAY_W / 2, Cfg::DISPLAY_H / 2 }, "YOU WIN!", 30);
	}

}