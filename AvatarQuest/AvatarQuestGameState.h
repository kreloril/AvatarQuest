#pragma once

#include "Common.h"
#include "Game.h"

namespace AvatarQuest {

// High-level game state identifiers
enum class AQStateId {
    None = 0,
    Title,
    MainMenu,
    CharCreation,
    Settings,
    World,
    Combat,
    Quit // sentinel to request application exit
};

// Minimal state interface used by AvatarQuestLayer to delegate
class IGameState {
public:
    virtual ~IGameState() = default;
    virtual void onEnter() {}
    virtual void onExit() {}
    virtual AQStateId handleEvents(float /*delta*/, Game::GameEvents& /*events*/) { return AQStateId::None; }
    virtual void update(float /*delta*/) {}
    virtual void render(float /*delta*/) {}
    virtual const char* name() const = 0;
};

}
