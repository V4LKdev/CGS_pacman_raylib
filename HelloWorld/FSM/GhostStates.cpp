#include "FSM/GhostStates.h"
#include "FSM/GhostState.h"
#include "Ghost.h"
#include "IGameBoard.h"
#include "Utils.h"

#include <vector>
#include <random>
#include <climits>
#include <cassert>

// GhostStates.h/cpp
// - Implements all concrete ghost states (Idle, Scatter, Chase, Frightened, Eaten)
// - Each state controls ghost behavior via OnEnter/OnExit/OnUpdate
// - Includes helper functions for movement decisions and path heuristics


#pragma region Helpers
namespace {

    // Canonical movement directions (up, right, down, left)
    const Play::Point2f DIRS[4] = {
        {0, -1}, // up
        {1, 0},  // right
        {0, 1},  // down
        {-1, 0}  // left
    };

    // Return opposite direction (used for frightened reversal)
    Play::Point2f OppositeDir(const Play::Point2f& d) { return Play::Point2f{ -d.x, -d.y }; }

    // Manhattan distance
    int DistI(int ax, int ay, int bx, int by) { return abs(ax - bx) + abs(ay - by); }

    // Return legal movement directions, avoiding walls and usually avoiding reversal
    std::vector<Play::Point2f> GetLegalDirs(const IGameBoard* board, int gx, int gy, const Play::Point2f& currDir) {
        std::vector<Play::Point2f> legal;
        Play::Point2f rev = OppositeDir(currDir);
        bool haveNonReverse = false;
        for (const Play::Point2f& d : DIRS) {
            int nx = gx + int(d.x), ny = gy + int(d.y);
            if (!board->IsWall(nx, ny)) {
                if (!(d.x == rev.x && d.y == rev.y)) { legal.push_back(d); haveNonReverse = true; }
            }
        }
        if (!haveNonReverse) {
            for (const Play::Point2f& d : DIRS) {
                int nx = gx + int(d.x), ny = gy + int(d.y);
                if (!board->IsWall(nx, ny)) legal.push_back(d);
            }
        }
        return legal;
    }

    // Choose direction that minimizes distance to target, using arcade tie-breaking
    Play::Point2f ChooseBestDir(int gx, int gy, int tx, int ty, const std::vector<Play::Point2f>& candidates)
    {
        if (candidates.empty()) return Play::Point2f{0,0};

        auto priorityIndex = [](const Play::Point2f& d)->int {
            if (d.x == 0 && d.y == -1) return 0; // up
            if (d.x == -1 && d.y == 0) return 1; // left
            if (d.x == 0 && d.y == 1) return 2;  // down
            return 3; // right
        };

        int bestScore = INT_MAX;
        int bestPriority = INT_MAX;
        std::vector<Play::Point2f> bests;

        for (const auto &d : candidates) {
            int nx = gx + int(d.x), ny = gy + int(d.y);
            int score = DistI(nx, ny, tx, ty);
            int pr = priorityIndex(d);
            if (score < bestScore) {
                bestScore = score;
                bestPriority = pr;
                bests.clear();
                bests.push_back(d);
            } else if (score == bestScore) {
                if (pr < bestPriority) {
                    bestPriority = pr;
                    bests.clear();
                    bests.push_back(d);
                } else if (pr == bestPriority) {
                    bests.push_back(d);
                }
            }
        }

        if (bests.size() == 1) return bests[0];

        // Randomly pick among equal candidates to reduce mechanical oscillation
        thread_local std::mt19937 rng{ std::random_device{}() };
        std::uniform_int_distribution<int> dist(0, static_cast<int>(bests.size()) - 1);
        return bests[dist(rng)];
    }

}
#pragma endregion

// ----------! Concrete states !----------

// --- Idle: ghost is frozen until game starts ---
class IdleState final : public GhostStateBase {
public:
    explicit IdleState(Ghost* g) : GhostStateBase(g) {}
    void OnEnter(IGameBoard* board) override {
        m_owner->dir = {0,0};
        m_owner->target = CenterOf(m_owner->gx, m_owner->gy);
        m_owner->speed = 0.0f;
    }
    void OnExit(IGameBoard* board) override { m_owner->speed = m_owner->baseSpeed; }
    void OnUpdate(IGameBoard* board, int pacGX, int pacGY, float dt) override { /* no auto transitions; Game triggers start */ }
    GhostState GetStateId() const override { return GhostState::Idle; }
    const char* GetName() const override { return "Idle"; }
};

// --- Scatter: ghost retreats to its corner of the maze ---
class ScatterState final : public GhostStateBase {
public:
    explicit ScatterState(Ghost* g) : GhostStateBase(g) {}
    void OnEnter(IGameBoard* board) override {
        m_owner->colour = m_owner->baseColour;
        m_owner->speed = m_owner->baseSpeed;
    }
    void OnUpdate(IGameBoard* board, int pacGX, int pacGY, float dt) override {
        if (!board) return;
        if (!AtCenter(m_owner->pos, m_owner->target)) return;
        int cgx = m_owner->gx, cgy = m_owner->gy;
        auto corner = board->GetScatterTarget(m_owner->type);
        int tx = corner.x, ty = corner.y;
        auto legal = GetLegalDirs(board, cgx, cgy, m_owner->dir);
        m_owner->dir = ChooseBestDir(cgx, cgy, tx, ty, legal);
        int ntx = cgx + int(m_owner->dir.x), nty = cgy + int(m_owner->dir.y);
        m_owner->target = !board->IsWall(ntx, nty) ? CenterOf(ntx, nty) : CenterOf(cgx, cgy);
    }
    GhostState GetStateId() const override { return GhostState::Scatter; }
    const char* GetName() const override { return "Scatter"; }
};

// --- Chase: ghost actively pursues Pac-Man with unique targeting rules ---
class ChaseState final : public GhostStateBase {
public:
    explicit ChaseState(Ghost* g) : GhostStateBase(g) {}
    void OnEnter(IGameBoard* board) override {
        m_owner->colour = m_owner->baseColour;
        m_owner->speed = m_owner->baseSpeed;
    }
    void OnUpdate(IGameBoard* board, int pacGX, int pacGY, float dt) override {
        if (!board) return;
        if (!AtCenter(m_owner->pos, m_owner->target)) return;
        int cgx = m_owner->gx, cgy = m_owner->gy;
        int tx = cgx, ty = cgy;

        switch (m_owner->type) {
            case GhostType::BLINKY:
                tx = pacGX; ty = pacGY; break;
            case GhostType::PINKY: {
                auto pacDir = board->GetPacDirection();
                tx = pacGX + int(pacDir.x) * 4;
                ty = pacGY + int(pacDir.y) * 4;
                break;
            }
            case GhostType::INKY: {
                auto pacDir = board->GetPacDirection();
                int pacAheadX = pacGX + int(pacDir.x) * 2;
                int pacAheadY = pacGY + int(pacDir.y) * 2;
                auto blinkyGrid = board->GetGhostGrid(GhostType::BLINKY);
                tx = pacAheadX * 2 - static_cast<int>(blinkyGrid.x);
                ty = pacAheadY * 2 - static_cast<int>(blinkyGrid.y);
                break;
            }
            case GhostType::CLYDE: {
                int dist = DistI(cgx, cgy, pacGX, pacGY);
                if (dist > Cfg::CLYDE_CHASE_SWITCH_DIST) { tx = pacGX; ty = pacGY; }
                else { auto corner = board->GetScatterTarget(m_owner->type); tx = corner.x; ty = corner.y; }
                break;
            }
        }

        auto legal = GetLegalDirs(board, cgx, cgy, m_owner->dir);
        m_owner->dir = ChooseBestDir(cgx, cgy, tx, ty, legal);
        int ntx = cgx + int(m_owner->dir.x), nty = cgy + int(m_owner->dir.y);
        m_owner->target = !board->IsWall(ntx, nty) ? CenterOf(ntx, nty) : CenterOf(cgx, cgy);
    }
    GhostState GetStateId() const override { return GhostState::Chase; }
    const char* GetName() const override { return "Chase"; }
};

// --- Frightened: ghost moves randomly, vulnerable to Pac-Man ---
class FrightenedState final : public GhostStateBase {
public:
    explicit FrightenedState(Ghost* g) : GhostStateBase(g) {}
    void OnEnter(IGameBoard* board) override {
        m_owner->colour = Play::cBlue;
        m_owner->speed = m_owner->baseSpeed * Cfg::FRIGHTENED_SPEED_MULT;
        m_owner->dir = OppositeDir(m_owner->dir);
    }
    void OnExit(IGameBoard* board) override {
        m_owner->colour = m_owner->baseColour;
        m_owner->speed = m_owner->baseSpeed;
    }
    void OnUpdate(IGameBoard* board, int pacGX, int pacGY, float dt) override {
        if (!board) return;
        if (!AtCenter(m_owner->pos, m_owner->target)) return;
        int cgx = m_owner->gx, cgy = m_owner->gy;
        auto legal = GetLegalDirs(board, cgx, cgy, m_owner->dir);
        if (legal.empty()) return;
        static thread_local std::mt19937 rng{ std::random_device{}() };
        std::uniform_int_distribution<int> dist(0, static_cast<int>(legal.size()) - 1);
        m_owner->dir = legal[dist(rng)];
        int ntx = cgx + int(m_owner->dir.x), nty = cgy + int(m_owner->dir.y);
        m_owner->target = !board->IsWall(ntx, nty) ? CenterOf(ntx, nty) : CenterOf(cgx, cgy);
    }
    GhostState GetStateId() const override { return GhostState::Frightened; }
    const char* GetName() const override { return "Frightened"; }
};

// --- Eaten: ghost returns to spawn to respawn ---
class EatenState final : public GhostStateBase {
public:
    explicit EatenState(Ghost* g) : GhostStateBase(g) {}
    void OnEnter(IGameBoard* board) override {
        m_owner->colour = Play::cWhite;
        m_owner->speed = m_owner->baseSpeed * Cfg::EATEN_SPEED_MULT;
    }
    void OnUpdate(IGameBoard* board, int pacGX, int pacGY, float dt) override {
        if (!board) return;
        if (!AtCenter(m_owner->pos, m_owner->target)) return;
        int cgx = m_owner->gx, cgy = m_owner->gy;
        int tx = m_owner->spawnGX, ty = m_owner->spawnGY;
        auto legal = GetLegalDirs(board, cgx, cgy, m_owner->dir);
        m_owner->dir = ChooseBestDir(cgx, cgy, tx, ty, legal);
        int ntx = cgx + int(m_owner->dir.x), nty = cgy + int(m_owner->dir.y);
        m_owner->target = !board->IsWall(ntx, nty) ? CenterOf(ntx, nty) : CenterOf(cgx, cgy);

        int curGX = int(m_owner->pos.x) / Cfg::TILE_SIZE;
        int curGY = int(m_owner->pos.y) / Cfg::TILE_SIZE;
        if (curGX == m_owner->spawnGX && curGY == m_owner->spawnGY) {
            m_owner->SetState(GhostState::Scatter, board);
        }
    }
    GhostState GetStateId() const override { return GhostState::Eaten; }
    const char* GetName() const override { return "Eaten"; }
};

// Factory implementations
std::unique_ptr<GhostStateBase> MakeIdleState(Ghost* owner) { return std::make_unique<IdleState>(owner); }
std::unique_ptr<GhostStateBase> MakeScatterState(Ghost* owner) { return std::make_unique<ScatterState>(owner); }
std::unique_ptr<GhostStateBase> MakeChaseState(Ghost* owner) { return std::make_unique<ChaseState>(owner); }
std::unique_ptr<GhostStateBase> MakeFrightenedState(Ghost* owner) { return std::make_unique<FrightenedState>(owner); }
std::unique_ptr<GhostStateBase> MakeEatenState(Ghost* owner) { return std::make_unique<EatenState>(owner); }

