#pragma once
#include <memory>
#include "FSM/GhostState.h"

class Ghost;

// Factory functions produce unique_ptrs to concrete states.
std::unique_ptr<GhostStateBase> MakeIdleState(Ghost* owner);
std::unique_ptr<GhostStateBase> MakeScatterState(Ghost* owner);
std::unique_ptr<GhostStateBase> MakeChaseState(Ghost* owner);
std::unique_ptr<GhostStateBase> MakeFrightenedState(Ghost* owner);
std::unique_ptr<GhostStateBase> MakeEatenState(Ghost* owner);
