#include "AvatarQuestLayer.h"

namespace AvatarQuest {

AvatarQuestLayer::AvatarQuestLayer() {}
AvatarQuestLayer::~AvatarQuestLayer() {}

void AvatarQuestLayer::init() {}
void AvatarQuestLayer::render(float /*deltaTime*/) {}
void AvatarQuestLayer::shutDown() {}

void AvatarQuestLayer::handleEvents(float /*delta*/, Game::GameEvents& events) {
    using Game::GameEvents;
    if (events.isEventType(GameEvents::EventType::KeyPress)) {
        // SDL keycodes are forwarded in keyEvent.keyCode
        if (events.keyEvent.keyCode == SDLK_ESCAPE) {
            Game::endGameLoop();
        }
    }
}

// Ensure we call the event handler from update so layer can react to events
void AvatarQuestLayer::update(float deltaTime) {
    Game::GameEvents& events = Game::getGameEvents();
    handleEvents(deltaTime, events);
}

Ref<Game::UILayer> CreateAvatarQuestLayer() {
    return CreateRef<AvatarQuestLayer>();
}

}
