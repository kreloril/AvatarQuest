
#include "common.h"
#include "AvatarQuest/AvatarQuest.h"

namespace Game {

    Ref<Game::UILayer> CreateInitialGameLayer(bool /*enableIMGui*/)
    {
        return AvatarQuest::CreateAvatarQuestLayer();
    }

}

