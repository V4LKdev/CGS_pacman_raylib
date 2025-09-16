// This file's header
#include "Ghost.h"

// Other includes
#include <climits>
#include <random>
#include "Game.h"

#pragma region Ghost Helpers

static const Play::Point2f DIRS[4] = {
	{0, -1}, // up
	{1, 0},  // right
	{0, 1},  // down
	{-1, 0}  // left
};

const char* ToString(const GhostState s)
{
    switch (s) {
    case GhostState::Scatter:    return "Scatter";
    case GhostState::Chase:      return "Chase";
    case GhostState::Frightened: return "Frightened";
    case GhostState::Eaten:      return "Eaten";
    }
    return "Unknown";
}

static inline Play::Point2f OppositeDir(const Play::Point2f& d)
{
	return { -d.x, -d.y };
}

static inline int DistI(int ax, int ay, int bx, int by)
{
	return abs(ax - bx) + abs(ay - by); // Manhattan distance
}

// Return list of legal directions from grid (gx,gy) avoiding immediate reverse
static std::vector<Play::Point2f> GetLegalDirs(const Game* game, const int gx, const int gy, const Play::Point2f& currDir) {
    std::vector<Play::Point2f> legal;
    const Play::Point2f rev = OppositeDir(currDir);
    bool haveNonReverse = false;

    for (const Play::Point2f& d : DIRS) {
        int nx = gx + int(d.x), ny = gy + int(d.y);
        if (!game->IsWall(nx, ny)) {
            // exclude reverse unless it's the only option
            if (!(d.x == rev.x && d.y == rev.y)) {
                legal.push_back(d);
                haveNonReverse = true;
            }
        }
    }

    // If no legal non-reverse dirs, allow reverse if that's open
    if (!haveNonReverse) {
        for (const Play::Point2f& d : DIRS) {
            int nx = gx + int(d.x), ny = gy + int(d.y);
            if (!game->IsWall(nx, ny)) legal.push_back(d);
        }
    }
    return legal;
}

// Choose the direction that minimizes Manhattan distance to target (tx,ty).
static Play::Point2f ChooseBestDir(int gx, int gy, int tx, int ty, const std::vector<Play::Point2f>& candidates)
{
    if (candidates.empty()) return Play::Point2f{0,0};


    auto priorityIndex = [](const Play::Point2f& d)->int {
        if (d.x == 0 && d.y == -1) return 0; // up
        if (d.x == -1 && d.y == 0) return 1; // left
        if (d.x == 0 && d.y == 1) return 2; // down
        return 3; // right
    };


    int bestScore = INT_MAX;
    Play::Point2f best = candidates[0];
    int bestPriority = INT_MAX;


    for (const auto &d : candidates) {
        int nx = gx + int(d.x), ny = gy + int(d.y);
        int score = DistI(nx, ny, tx, ty);
        int pr = priorityIndex(d);
        if (score < bestScore || (score == bestScore && pr < bestPriority)) {
            bestScore = score; best = d; bestPriority = pr;
        }
    }
    return best;
}

#pragma endregion

void Ghost::Init(GhostType t, int startGX, int startGY, Play::Colour col)
{
	type = t;
    gx = spawnGX = startGX;
    gy = spawnGY = startGY;
	pos = target = CenterOf(gx, gy);
	dir = {0, 0};
	colour = col;
	baseColour = col;
    speed = baseSpeed;
    state = GhostState::Idle;
}


void Ghost::EnterFrightened()
{
    if (state == GhostState::Eaten) return;
    state = GhostState::Frightened;
    colour = Play::cBlue;
    speed = baseSpeed * 0.7f; // half speed when frightened

    dir = OppositeDir(dir);
}

void Ghost::ExitFrightened()
{
    if (state != GhostState::Frightened) return;
    colour = baseColour;
    speed = baseSpeed;
    state = GhostState::Scatter;
}

void Ghost::SetEaten()
{
    state = GhostState::Eaten;
    colour = Play::cWhite;
    speed = baseSpeed * 1.6f;
}

void Ghost::OnGlobalModeChange(const GlobalMode newMode)
{
    if (state == GhostState::Scatter || state == GhostState::Chase)
    {
        state = (newMode == GlobalMode::Scatter) ? GhostState::Scatter : GhostState::Chase;
    }
}

void Ghost::ResetToSpawn()
{
    Init(type, spawnGX, spawnGY, baseColour);
}


Play::Point2f Ghost::TickAI(const Game* game, int pacGX, int pacGY) const
{
	// current grid coords
    int cgx = gx, cgy = gy;

    // Idle: no movement
    if (state == GhostState::Idle) {
        return Play::Point2f{0,0};
    }

    // Eaten: target spawn position
    if (state == GhostState::Eaten) {
        int tx = spawnGX, ty = spawnGY;
        auto legal = GetLegalDirs(game, cgx, cgy, dir);
        return ChooseBestDir(cgx, cgy, tx, ty, legal);
    }

    // Frightened: random legal direction (include reverse for randomness)
    if (state == GhostState::Frightened) {
        std::vector<Play::Point2f> legal;
        for (const Play::Point2f& d : DIRS) {
            int nx = cgx + int(d.x), ny = cgy + int(d.y);
            if (!game->IsWall(nx, ny)) legal.push_back(d);
        }
        if (legal.empty()) return Play::Point2f{0,0};


        // Use local RNG (if you prefer to use a shared RNG from Game, replace this block)
        static std::random_device rd;
        static std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dist(0, static_cast<int>(legal.size()) - 1);
        int idx = dist(rng);
        return legal[idx];
    }

    // Scatter / Chase: compute a target tile, then pick best direction
    int tx = cgx, ty = cgy;

    if (state == GhostState::Scatter) {
        auto corner = game->GetScatterTarget(type);
        tx = corner.x; ty = corner.y;
    }
    else if (state == GhostState::Chase) {
        switch (type) {
        case GhostType::BLINKY:
            // target Pac-Man's current tile
            tx = pacGX; ty = pacGY;
            break;
        case GhostType::PINKY:
            {
                // Ambusher: target 4 tiles ahead of Pac's direction
                auto pacDir = game->GetPacDirection();
                tx = pacGX + int(pacDir.x) * 4;
                ty = pacGY + int(pacDir.y) * 4;
                break;
            }
        case GhostType::INKY:
            {
                // Flanker: uses Blinky & Pac
                auto pacDir = game->GetPacDirection();
                int pacAheadX = pacGX + int(pacDir.x) * 2;
                int pacAheadY = pacGY + int(pacDir.y) * 2;
                auto blinkyGrid = game->GetGhostGrid(GhostType::BLINKY);
                tx = pacAheadX * 2 - blinkyGrid.x;
                ty = pacAheadY * 2 - blinkyGrid.y;
                break;
            }
        case GhostType::CLYDE:
            {
                int dist = DistI(cgx, cgy, pacGX, pacGY);
                if (dist > 8) { tx = pacGX; ty = pacGY; }
                else { auto corner = game->GetScatterTarget(type); tx = corner.x; ty = corner.y; }
                break;
            }
        }
    }

    auto legal = GetLegalDirs(game, cgx, cgy, dir);
    return ChooseBestDir(cgx, cgy, tx, ty, legal);
}

void Ghost::Update(Game* game, int pacGX, int pacGY, float dt)
{
    // Decide next direction when centered on tile
    if (AtCenter(pos, target))
    {
        gx = int(pos.x) / Cfg::TILE_SIZE;
        gy = int(pos.y) / Cfg::TILE_SIZE;

        Play::Point2f next = TickAI(game, pacGX, pacGY);

        int nx = gx + int(next.x), ny = gy + int(next.y);
        if (!game->IsWall(nx, ny))
        {
            dir = next;
        }
        else
        {
            int fx = gx + int(dir.x), fy = gy + int(dir.y);
            if (game->IsWall(fx, fy))
            {
                dir = {0, 0};
            }
        }

        int tx = gx + int(dir.x), ty = gy + int(dir.y);
        target = !game->IsWall(tx, ty) ? CenterOf(tx, ty) : CenterOf(gx, gy);
    }

    // Movement
    Play::Point2f d{ target.x - pos.x, target.y - pos.y };
    float dist = std::sqrt(d.x * d.x + d.y * d.y);
    float step = speed * dt;

    if (dist < 0.0001f || step >= dist)
    {
        pos = target;
    }
    else
    {
        pos.x += (d.x / dist) * step;
        pos.y += (d.y / dist) * step;
    }

    // If ghost is Eaten and reached spawn, respawn to Scatter
    if (state == GhostState::Eaten)
    {
        int curGX = int(pos.x) / Cfg::TILE_SIZE;
        int curGY = int(pos.y) / Cfg::TILE_SIZE;
        if (curGX == spawnGX && curGY == spawnGY) {
            // Arrived at spawn
            state = GhostState::Scatter;
            colour = baseColour;
            speed = baseSpeed;
            // place exactly at center
            pos = target = CenterOf(spawnGX, spawnGY);
            dir = {0,0};
        }
    }
}

void Ghost::Draw() const
{
	Play::DrawCircle(pos, Cfg::TILE_SIZE / 2 - 2, colour);

    Play::Point2f textPos = { pos.x, pos.y - Cfg::TILE_SIZE };
    DrawDebugText(textPos, ToString(state), 14, Play::cWhite);
}