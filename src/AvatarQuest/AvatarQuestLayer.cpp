#include "AvatarQuest/AvatarQuestLayer.h"

namespace AvatarQuest {

AvatarQuestLayer::AvatarQuestLayer() {}
AvatarQuestLayer::~AvatarQuestLayer() {}

void AvatarQuestLayer::init() {}
void AvatarQuestLayer::update(float /*deltaTime*/) {}
void AvatarQuestLayer::render(float /*deltaTime*/) {}
void AvatarQuestLayer::shutDown() {}
void AvatarQuestLayer::handleEvents(float /*delta*/, Game::GameEvents& /*events*/) {}

Ref<Game::UILayer> CreateAvatarQuestLayer() {
    return CreateRef<AvatarQuestLayer>();
}

}
