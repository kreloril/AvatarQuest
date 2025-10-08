#include "common.h"
#include <SDL3/SDL_main.h>

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{

    (void)appstate; (void)argc; (void)argv;

    //  auto gameLayer = CreateRef<Editor::EditorGameLayer>();
    //  Ref<Game::UILayer> uilayer = DynamicCast<Game::UILayer, Ref<Editor::EditorGameLayer>>(gameLayer);

    auto uilayer = Game::CreateInitialGameLayer(true);

    if (!Game::initGame("settings.json")) {
        std::cout << "Game initialization failed." << std::endl;
        return SDL_APP_FAILURE;
    }
    uilayer->init();
    Game::addlayer(uilayer);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;

    ImGui_ImplSDL3_InitForSDLRenderer(Window::getWindowHandle(), Renderer::getRenderer());
    ImGui_ImplSDLRenderer3_Init(Renderer::getRenderer());

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    if (!Game::runGameLoop())
        return SDL_APP_SUCCESS;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    // Handle SDL events here if needed
    Game::handleEvent(event);
    ImGui_ImplSDL3_ProcessEvent(event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    // Clean up
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    Game::shutDownGame();

}


namespace Game {




}


