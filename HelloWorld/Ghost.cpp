// This file's header
#include "Ghost.h"

// Other includes
#include "Game.h"

void Ghost::Init(GhostType t, int startGX, int startGY, Play::Colour col)
{
	type = t; gx = startGX; gy = startGY;
	pos = target = CenterOf(gx, gy);
	dir = { 0,0 };
	colour = col;
}

Play::Point2f Ghost::TickAI(const Game* game, int pacGX, int pacGY) const
{
	// Todo - ADD A STATE MACHINE HERE!
	// This function must return a GRID DIRECTION vector:
	//   {  1,  0 } = move right
	//   { -1,  0 } = move left
	//   {  0,  1 } = move down
	//   {  0, -1 } = move up
	//   {  0,  0 } = don't move (stay in place)

	/* e.g.
	switch (CurrentState)
	{
		case AIState::SEARCH :
		{
			// Pick a random point in the arena to go to
			// if we get with x tiles of the player, transition to CHASE
			// if the player gets the power up, transition to EVADE
			break;
		}
		case AIState::CHASE:
		{
			// Move toward the players location
			// if the player moves more than x spaces away, transition to SEARCH
			// if the player gets the power up, transition to EVADE
			break;
		}
		case AIState::EVADE:
		{
			// Move directly away from the player
			// when the player's power-up expires, transition to SEARCH
			break;
		}
	}
	*/

	return Play::Point2f();
}

void Ghost::Update(Game* game, int pacGX, int pacGY, float dt)
{
	if (AtCenter(pos, target))
	{
		gx = int(pos.x) / Cfg::TILE_SIZE;
		gy = int(pos.y) / Cfg::TILE_SIZE;

		Play::Point2f next = TickAI(game, pacGX, pacGY);

		int nx = gx + int(next.x), ny = gy + int(next.y);
		if (!game->IsWall(nx, ny))
			dir = next;
		else
		{
			// blocked in chosen dir; stop if forward also blocked
			int fx = gx + int(dir.x), fy = gy + int(dir.y);
			if (game->IsWall(fx, fy))
			{
				dir = { 0,0 };
			}
		}

		int tx = gx + int(dir.x), ty = gy + int(dir.y);
		target = !game->IsWall(tx, ty) ? CenterOf(tx, ty) : CenterOf(gx, gy);
	}

	Play::Point2f d{ target.x - pos.x, target.y - pos.y };
	float dist = std::sqrt(d.x * d.x + d.y * d.y);
	float step = speed * dt;

	if (dist < 0.0001f || step >= dist) 
	{ 
		pos = target; return; 
	}

	pos.x += (d.x / dist) * step;
	pos.y += (d.y / dist) * step;
}

void Ghost::Draw() const
{
	Play::DrawCircle(pos, Cfg::TILE_SIZE / 2 - 2, colour);
}