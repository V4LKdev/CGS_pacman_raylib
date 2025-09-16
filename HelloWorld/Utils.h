#pragma once

#include "RaylibPlayCompat.h"

struct Cfg
{
        static constexpr int TILE_SIZE = 16;
        static constexpr int GRID_WIDTH = 28;
        static constexpr int GRID_HEIGHT = 31;
        static constexpr int DISPLAY_W = GRID_WIDTH * TILE_SIZE;
        static constexpr int DISPLAY_H = GRID_HEIGHT * TILE_SIZE;
        static constexpr int DISPLAY_SCALE = 2;
        // Duration that a power-up remains active in seconds
        static constexpr float POWERUP_DURATION = 5.0f;

        // Ghost state durations
        static constexpr float CHASE_DURATION = 20.0f;
        static constexpr float SCATTER_DURATION = 7.0f;

        // Ghost speed multipliers
        static constexpr float FRIGHTENED_SPEED_MULT = 0.7f;
        static constexpr float EATEN_SPEED_MULT = 1.6f;
        static constexpr float BASE_GHOST_SPEED = 80.0f;

        // Pacman speed
        static constexpr float PACMAN_SPEED = 90.0f;

        // Drawing sizes
        static constexpr int PELLET_RADIUS = 2;
        static constexpr int POWERUP_RADIUS = 5;
        static constexpr int ACTOR_DRAW_INSET = 2; // inset from half tile for actors (Pacman, Ghosts)

        // Gameplay thresholds
        static constexpr float EPS = 1e-4f; // movement snap epsilon
        static constexpr int CLYDE_CHASE_SWITCH_DIST = 8; // tiles

        // Collision tuning
        static constexpr float COLLISION_PAD = 2.0f; // forgiving extra pixels added to sum of radii

        static constexpr bool DEBUG_MODE = true;
};

enum class TileType {
        WALL,
        EMPTY,
        PELLET,
        POWERUP
};

inline Play::Point2f CenterOf(const int gx, const int gy)
{
    return {gx * Cfg::TILE_SIZE + Cfg::TILE_SIZE / 2,
            gy * Cfg::TILE_SIZE + Cfg::TILE_SIZE / 2};
}

inline bool AtCenter(const Play::Point2f& a, const Play::Point2f& b)
{
    const float eps = Cfg::EPS;
    return (std::abs(a.x - b.x) < eps) && (std::abs(a.y - b.y) < eps);
}

inline float Distance(const Play::Point2f& a, const Play::Point2f& b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

inline float ActorRadius()
{
    return Cfg::TILE_SIZE * 0.5f - static_cast<float>(Cfg::ACTOR_DRAW_INSET);
}

inline bool WithinRadius(const Play::Point2f& a, const Play::Point2f& b, float radius)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return (dx*dx + dy*dy) <= (radius*radius);
}
