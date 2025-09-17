#pragma once

class IGameBoard;
class Ghost;

// GhostState.h
// - Defines GhostState enum for symbolic AI modes
// - Provides GhostStateBase: an abstract base class for all ghost states
// - Each state implements lifecycle hooks (OnEnter/OnExit/OnUpdate) and identifiers


enum class GhostState
{
    Idle,
    Scatter,
    Chase,
    Frightened,
    Eaten
};

// Base class for all ghost state objects.
class GhostStateBase
{
protected:
    Ghost* m_owner; // Owning ghost (guaranteed valid, FSM owned by Ghost)
public:
    explicit GhostStateBase(Ghost* owner) : m_owner(owner) {}
    virtual ~GhostStateBase() = default;

    // Lifecycle hooks:
    // - OnEnter: called when state is activated
    // - OnExit: called when state is deactivated
    // - OnUpdate: called every frame while active
    virtual void OnEnter(IGameBoard* board) {}
    virtual void OnExit(IGameBoard* board) {}
    virtual void OnUpdate(IGameBoard* board, int pacGX, int pacGY, float dt) {}

    // Identification (must be implemented by each concrete state)
    virtual GhostState GetStateId() const = 0;
    virtual const char* GetName() const = 0;
};
