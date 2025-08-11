#pragma once

#include "Play.h"

struct Cfg
{
	static constexpr int TILE_SIZE = 16;
	static constexpr int GRID_WIDTH = 28;
	static constexpr int GRID_HEIGHT = 31;
	static constexpr int DISPLAY_W = GRID_WIDTH * TILE_SIZE;
	static constexpr int DISPLAY_H = GRID_HEIGHT * TILE_SIZE;
	static constexpr int DISPLAY_SCALE = 2;
};

enum TileType { 
	TILE_WALL,
	TILE_EMPTY,
	TILE_PELLET 
};

inline Play::Point2f CenterOf(int gx, int gy)
{
	return { gx * Cfg::TILE_SIZE + Cfg::TILE_SIZE / 2,
			 gy * Cfg::TILE_SIZE + Cfg::TILE_SIZE / 2 };
}

inline bool AtCenter(const Play::Point2f& a, const Play::Point2f& b)
{
	const float eps = 0.01f;
	return (std::abs(a.x - b.x) < eps) && (std::abs(a.y - b.y) < eps);
}