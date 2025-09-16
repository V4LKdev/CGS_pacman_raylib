// This file's header
#include "Game.h"

// Other includes
#include "Ghost.h"
#include "Pacman.h"
#include <random>

Game::Game()
{
	pac = new Pacman();

	ghosts.resize(4);
	for (int i = 0; i <= 3; ++i)
	{
		ghosts[i] = new Ghost();
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

	return maze[y][x] == TILE_WALL;
}

void Game::BuildArena()
{
	for (int y = 0; y < Cfg::GRID_HEIGHT; ++y)
	{
		for (int x = 0; x < Cfg::GRID_WIDTH; ++x)
		{
			const bool border = (x == 0 || y == 0 || x == Cfg::GRID_WIDTH - 1 || y == Cfg::GRID_HEIGHT - 1);
			maze[y][x] = border ? TILE_WALL : TILE_PELLET;
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
			if (maze[y][x] == TILE_PELLET)
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
		maze[static_cast<int>(choice.x)][static_cast<int>(choice.x)] = TILE_POWERUP;
		powerUpPresent = true;
	}
}

void Game::ActivatePowerUp()
{
	powerUpTimer = Cfg::POWERUP_DURATION;
	powerUpPresent = false;
	for (auto* g : ghosts)
	{
		g->colour = Play::cBlue;
		// TODO: implement state change here
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
	ghosts[0]->Init(BLINKY, cx - 2, cy, Play::cRed);
	ghosts[1]->Init(INKY, cx, cy, Play::cCyan);
	ghosts[2]->Init(PINKY, cx + 2, cy, Play::cMagenta);
	ghosts[3]->Init(CLYDE, cx, cy + 2, Play::cOrange);

	SpawnPowerUp();
}

void Game::DrawMaze() const
{
	for (int y = 0; y < Cfg::GRID_HEIGHT; ++y)
	{
		for (int x = 0; x < Cfg::GRID_WIDTH; ++x)
		{
			int px = x * Cfg::TILE_SIZE, py = y * Cfg::TILE_SIZE;
			if (maze[y][x] == TILE_WALL)
			{
				Play::DrawRect({ px, py }, { px + Cfg::TILE_SIZE - 1, py + Cfg::TILE_SIZE - 1 }, Play::cBlue, true);
			}				
			else if (maze[y][x] == TILE_PELLET)
			{
				Play::DrawCircle({ px + Cfg::TILE_SIZE / 2, py + Cfg::TILE_SIZE / 2 }, 2, Play::cWhite);
			}				
			else if (maze[y][x] == TILE_POWERUP)
			{
				Play::DrawCircle({ px + Cfg::TILE_SIZE / 2, py + Cfg::TILE_SIZE / 2 }, 5, Play::cYellow);
			}				
		}
	}

}

void Game::Update(float dt)
{
	pac->HandleInput();
	pac->Update(this, dt);

	if (powerUpTimer > 0.0f)
	{
		powerUpTimer -= dt;
		if (powerUpTimer <= 0.0f)
		{
			powerUpTimer = 0.0f;
			for (auto* g : ghosts)
			{
				g->colour = g->baseColour;
				// TODO: implement state change here
			}				
		}
	}
	else if (!powerUpPresent)
	{
		// Power-up expired and none present on the board
		SpawnPowerUp();
	}

	for (auto* g : ghosts)
	{
		g->Update(this, pac->gx, pac->gy, dt);
	}		
}

void Game::Draw() const
{
	DrawMaze();
	pac->Draw();

	for (auto* g : ghosts)
	{
		g->Draw();
	}

	// Win text
	bool pelletsLeft = false;
	for (int y = 0; y < Cfg::GRID_HEIGHT && !pelletsLeft; ++y)
	{
		for (int x = 0; x < Cfg::GRID_WIDTH; ++x)
		{
			if (maze[y][x] == TILE_PELLET) pelletsLeft = true;
		}
	}

	if (!pelletsLeft)
	{
		Play::DrawDebugText({ Cfg::DISPLAY_W / 2, Cfg::DISPLAY_H / 2 }, "YOU WIN!");
	}
}