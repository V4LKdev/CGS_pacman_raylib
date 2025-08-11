// This file's header
#include "Pacman.h"

// Other includes
#include "Game.h"

void Pacman::Init(int startGX, int startGY)
{
	gx = startGX; gy = startGY;
	pos = target = CenterOf(gx, gy);
	dir = queued = { 0,0 };
}

void Pacman::HandleInput()
{
	if (Play::KeyDown(Play::KEY_UP))
	{
		queued = { 0, 1 };
	}
	if (Play::KeyDown(Play::KEY_DOWN))
	{
		queued = { 0,-1 };
	}
	if (Play::KeyDown(Play::KEY_LEFT))
	{
		queued = { -1, 0 };
	}
	if (Play::KeyDown(Play::KEY_RIGHT))
	{
		queued = { 1, 0 };
	}
}

void Pacman::StepTowards(const Play::Point2f& tgt, float dt)
{
	Play::Point2f d{ tgt.x - pos.x, tgt.y - pos.y };
	float dist = std::sqrt(d.x * d.x + d.y * d.y);
	float step = speed * dt;
	if (dist < 0.0001f || step >= dist) { pos = tgt; return; }
	pos.x += (d.x / dist) * step;
	pos.y += (d.y / dist) * step;
}

void Pacman::Update(Game* game, float dt)
{
	if (AtCenter(pos, target))
	{
		gx = int(pos.x) / Cfg::TILE_SIZE;
		gy = int(pos.y) / Cfg::TILE_SIZE;

		// Eat pellet or power-up
		if (game->InBounds(gx, gy))
		{
			if (game->maze[gy][gx] == TILE_PELLET)
			{
				game->maze[gy][gx] = TILE_EMPTY;
			}
			else if (game->maze[gy][gx] == TILE_POWERUP)
			{
				game->maze[gy][gx] = TILE_EMPTY;
				game->ActivatePowerUp();
			}
		}

		// Try queued first
		int qx = gx + int(queued.x), qy = gy + int(queued.y);
		if ((queued.x || queued.y) && !game->IsWall(qx, qy))
		{
			dir = queued;
		}
		else
		{
			int cx = gx + int(dir.x), cy = gy + int(dir.y);
			if (game->IsWall(cx, cy))
			{
				dir = { 0,0 };
			}
		}

		// Next target
		int tx = gx + int(dir.x), ty = gy + int(dir.y);
		target = !game->IsWall(tx, ty) ? CenterOf(tx, ty) : CenterOf(gx, gy);
	}

	StepTowards(target, dt);
}

void Pacman::Draw() const
{
	Play::DrawCircle(pos, Cfg::TILE_SIZE / 2 - 2, Play::cYellow);
}