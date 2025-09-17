#include "FSM/GhostStateMachine.h"
#include "FSM/GhostState.h"
#include "Ghost.h"
#include <cassert>

GhostStateMachine::GhostStateMachine(Ghost* owner)
    : m_owner(owner), m_currentStateEnum(GhostState::Idle), m_currentStatePtr(nullptr)
{
    assert(owner && "GhostStateMachine requires a non-null owner");
}

void GhostStateMachine::AddState(GhostState id, std::unique_ptr<GhostStateBase> state)
{
    assert(state);
    auto res = m_states.emplace(id, std::move(state));
    assert(res.second && "AddState: state already registered for this id");
}

void GhostStateMachine::SetState(GhostState newState, IGameBoard* board)
{
    // Prevent Eaten → Frightened (illegal in Pac-Man rules)
    if (m_currentStatePtr && m_currentStateEnum == GhostState::Eaten && newState == GhostState::Frightened) {
        return;
    }

    // Skip redundant transitions
    if (m_currentStatePtr && newState == m_currentStateEnum) return;

    // Exit old state
    if (m_currentStatePtr) m_currentStatePtr->OnExit(board);

    // Look up new state
    auto it = m_states.find(newState);
    m_currentStateEnum = newState;
    m_currentStatePtr = (it != m_states.end()) ? it->second.get() : nullptr;

    // Enter new state if it exists
    if (m_currentStatePtr) m_currentStatePtr->OnEnter(board);
}

void GhostStateMachine::Update(IGameBoard* board, int pacGX, int pacGY, float dt) const
{
    if (m_currentStatePtr) m_currentStatePtr->OnUpdate(board, pacGX, pacGY, dt);
}

GhostState GhostStateMachine::GetCurrentState() const { return m_currentStateEnum; }

const char* GhostStateMachine::GetCurrentStateName() const
{
    return m_currentStatePtr ? m_currentStatePtr->GetName() : "None";
}

