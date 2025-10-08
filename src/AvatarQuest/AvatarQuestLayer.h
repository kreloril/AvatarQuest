#pragma once
#include "Common.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

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

private:
    SDL_Texture* m_texture = nullptr;
    float m_texW = 0.0f, m_texH = 0.0f;
};

Ref<Game::UILayer> CreateAvatarQuestLayer();

}
