#include "common.h"
#include "Game.h"
#include "AvatarQuestLayer.h"
#include "common.h"
#include "Game.h"

namespace Game {

    Ref<Game::UILayer> CreateInitialGameLayer(bool a)
    {
        return AvatarQuest::CreateAvatarQuestLayer();
    }

}
int main(int argc, char** argv)
