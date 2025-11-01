#pragma once
#include "Common.h"
#include "AvatarQuestGameState.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifdef AVATARQUEST_ENABLE_AUDIO
// Forward declaration to avoid including Sound.h here
namespace Sound { struct Music; }
#endif


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
	// Active game state
	AvatarQuest::AQStateId _stateId = AvatarQuest::AQStateId::Title;
	std::shared_ptr<AvatarQuest::IGameState> _state;

    // Shared background image for non-World screens
    Ref<Renderer::Image> _bgImage;

#ifdef AVATARQUEST_ENABLE_AUDIO
    // Menu background music (played on Title/MainMenu/CharCreation/Settings)
    Sound::Music* _menuMusic = nullptr;
    bool _menuMusicPlaying = false;
#endif

	void setState(AvatarQuest::AQStateId id);
};

Ref<Game::UILayer> CreateAvatarQuestLayer();

}

#include "AvatarQuestRP.h"

