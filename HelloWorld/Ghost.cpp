#include "Ghost.h"
#include "IGameBoard.h"
#include "FSM/GhostStates.h"
#include "Modes.h"

#pragma region Helpers

const char* ToString(GhostState s)
{
    switch (s) {
    case GhostState::Idle: return "Idle";
    case GhostState::Scatter: return "Scatter";
    case GhostState::Chase: return "Chase";
    case GhostState::Frightened: return "Frightened";
    case GhostState::Eaten: return "Eaten";
    }
    return "Unknown";
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
    baseSpeed = Cfg::BASE_GHOST_SPEED;
    speed = baseSpeed;

    InitStateMachine();
}

void Ghost::InitStateMachine()
{
    m_fsm = std::make_unique<GhostStateMachine>(this);

    m_fsm->AddState(GhostState::Idle, MakeIdleState(this));
    m_fsm->AddState(GhostState::Scatter, MakeScatterState(this));
    m_fsm->AddState(GhostState::Chase, MakeChaseState(this));
    m_fsm->AddState(GhostState::Frightened, MakeFrightenedState(this));
    m_fsm->AddState(GhostState::Eaten, MakeEatenState(this));

    // Set initial state
    m_fsm->SetState(GhostState::Idle, nullptr);
}

void Ghost::SetState(GhostState newState, IGameBoard* board)
{
    if (m_fsm) m_fsm->SetState(newState, board);
}

GhostState Ghost::GetState() const
{
    if (m_fsm) return m_fsm->GetCurrentState();
    return GhostState::Idle;
}

// Event APIs
void Ghost::EnterFrightened(IGameBoard* board)
{
    if (GetState() == GhostState::Eaten) return;
    SetState(GhostState::Frightened, board);
}

void Ghost::ExitFrightened(IGameBoard* board)
{
    if (GetState() != GhostState::Frightened) return;
    SetState(board && board->GetGlobalMode() == GlobalMode::Scatter ? GhostState::Scatter : GhostState::Chase, board);
}

void Ghost::SetEaten(IGameBoard* board)
{
    SetState(GhostState::Eaten, board);
}

void Ghost::OnGlobalModeChange(IGameBoard* board, GlobalMode newMode)
{
    if (GetState() == GhostState::Eaten || GetState() == GhostState::Frightened || GetState() == GhostState::Idle) return;

    newMode == GlobalMode::Scatter ? SetState(GhostState::Scatter, board) : SetState(GhostState::Chase, board);
}

void Ghost::ResetToSpawn()
{
    gx = spawnGX; gy = spawnGY;
    pos = target = CenterOf(gx, gy);
    dir = {0,0};
    colour = baseColour;
    speed = baseSpeed;
    m_fsm->SetState(GhostState::Idle, nullptr);
}

bool Ghost::Update(IGameBoard* board, int pacGX, int pacGY, float dt)
{
    // Update FSM
    if (m_fsm) m_fsm->Update(board, pacGX, pacGY, dt);

    // Movement code
    if (AtCenter(pos, target))
    {
        gx = int(pos.x) / Cfg::TILE_SIZE;
        gy = int(pos.y) / Cfg::TILE_SIZE;

        int tx = gx + int(dir.x), ty = gy + int(dir.y);
        if (board && board->IsWall(tx, ty)) dir = {0,0};
        target = (board && !board->IsWall(tx, ty)) ? CenterOf(tx, ty) : CenterOf(gx, gy);
    }

    Play::Point2f d{ target.x - pos.x, target.y - pos.y };
    float dist = Distance(pos, target);
    float step = speed * dt;

    if (dist < Cfg::EPS || step >= dist) pos = target;
    else
    {
        pos.x += (d.x / dist) * step;
        pos.y += (d.y / dist) * step;
    }

    // Check collision with Pac-Man - only if not in Eaten state
    if (GetState() != GhostState::Eaten && board)
    {
        const Play::Point2f pacPos = board->GetPacPosition();
        const float ghostR = ActorRadius();
        const float pacR = ActorRadius();
        const float totalR = ghostR + pacR + Cfg::COLLISION_PAD;
        if (WithinRadius(pos, pacPos, totalR))
        {
            return true; // Collision detected
        }
    }

    return false; // No collision
}

void Ghost::Draw() const
{
    Play::DrawCircle(pos, Cfg::TILE_SIZE / 2 - Cfg::ACTOR_DRAW_INSET, colour);
    Play::Point2f textPos = { pos.x, pos.y - Cfg::TILE_SIZE };

    if (Cfg::DEBUG_MODE)
    {
        DrawDebugText(textPos, ToString(GetState()), 14, Play::cWhite);
    }
}
