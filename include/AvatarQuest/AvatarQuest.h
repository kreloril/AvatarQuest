#pragma once

// Public header for the AvatarQuest module. Exposes a small factory used by
// the engine entry point to create the default gameplay/UI layer.

#include <memory>

namespace Game { class UILayer; }

namespace AvatarQuest {

std::shared_ptr<Game::UILayer> CreateAvatarQuestLayer();

}
