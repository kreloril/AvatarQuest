#pragma once
#include "Common.h"

namespace AvatarQuest {

class AvatarQuestLayer : public Game::UILayer {
public:
    AvatarQuestLayer();
    ~AvatarQuestLayer() override;

    void init() override;
    void update(float deltaTime) override;
    void render(float deltaTime) override;
    void shutDown() override;
    void handleEvents(float delta, Game::GameEvents& events) override;
};

Ref<Game::UILayer> CreateAvatarQuestLayer();

}
