
#include "common.h"
#include "AvatarQuestLayer.h"

namespace Game {

    Ref<Game::UILayer> CreateInitialGameLayer(bool /*enableIMGui*/)
    {
        return AvatarQuest::CreateAvatarQuestLayer();
    }

}

