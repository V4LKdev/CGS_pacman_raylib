#pragma once

#include <map>
#include <memory>
#include "FSM/GhostState.h"

class Ghost;
class GhostStateBase;
class IGameBoard;

// GhostStateMachine:
// - Manages a single ghost's AI states and transitions.
// - Stores owned state instances and delegates per-frame behavior.
// - Uses enum + pointer combo for fast access and safe transitions.
class GhostStateMachine
{
public:
    explicit GhostStateMachine(Ghost* owner);
    ~GhostStateMachine() = default;

    // non-copyable
    GhostStateMachine(const GhostStateMachine&) = delete;
    GhostStateMachine& operator=(const GhostStateMachine&) = delete;

    // Register a new state with unique ownership
    // Duplicate registration triggers assert
    void AddState(GhostState id, std::unique_ptr<GhostStateBase> state);
    // SetState handles full transition:
    // 1) Guard illegal transitions (e.g., Eaten → Frightened)
    // 2) Skip if already in state
    // 3) Call OnExit on current state
    // 4) Swap pointer and enum
    // 5) Call OnEnter on new state
    void SetState(GhostState newState, IGameBoard* board);
    // Delegate tick update to current state
    // FSM container is const. states modify Ghost and query IGameBoard
    void Update(IGameBoard* board, int pacGX, int pacGY, float dt) const;

    GhostState GetCurrentState() const;
    const char* GetCurrentStateName() const;

private:
    Ghost* m_owner;
    std::map<GhostState, std::unique_ptr<GhostStateBase>> m_states;
    GhostState m_currentStateEnum;
    GhostStateBase* m_currentStatePtr;
};
