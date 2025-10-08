## AvatarQuest — AI assistant guide

Purpose: quickly orient an AI coding agent to the codebase, conventions, build/run workflow, and integration points so changes are consistent and low-risk.

- Top-level facts
  - Language: C (C17). Build system: CMake (minimum 3.24). External dependency: SDL3 (found via CMake `find_package(SDL3 CONFIG REQUIRED)`).
  - Single executable project (target `AvatarQuest`) built from `src/*.c` and public headers under `include/AvatarQuest`.

- Big-picture architecture (what to edit and why)
  - `src/main.c` — bootstraps the app: calls `avatarquest_platform_initialize()`, creates the `AvatarQuestGame` state, runs the platform main loop, then cleans up.
  - `src/platform.c` — platform layer: initializes SDL, creates the window and renderer, owns the main loop (`avatarquest_platform_mainloop`) and low-level event polling. Keep platform-specific code here (windowing, audio, VSync, renderer lifetime).
  - `src/game.c` — gameplay layer: `AvatarQuestGame` container and frame logic (create/destroy/update/render/handle_event). This file should remain renderer- and platform-agnostic wherever possible; rendering is done through the renderer attached by `avatarquest_game_attach_renderer()`.
  - Separation rationale: platform handles SDL lifecycle and frame timing; game manages state and draws using the provided renderer. Follow this separation when adding features.

- Code-patterns and conventions (concrete examples)
  - Naming: public functions and symbols use the `avatarquest_` prefix (e.g. `avatarquest_game_create`, `avatarquest_platform_initialize`). Structs use `AvatarQuest*` (e.g. `AvatarQuestGame`). Match these exactly for consistency.
  - Memory: code uses SDL allocation helpers (e.g. `SDL_calloc`, `SDL_free`). Prefer these for allocations tied to SDL lifetime when modifying code.
  - Safety: functions defensively check for NULL; propagate this pattern in new modules.
  - Event handling: platform polls SDL events and forwards them to `avatarquest_game_handle_event()` (see `src/platform.c`); keep input parsing in game layer or add precise platform-level events if strictly platform-only.
  - Rendering flow: call `avatarquest_game_attach_renderer(game, renderer)` once before the loop; during each frame call `avatarquest_game_update()` then `avatarquest_game_render()` (see `src/platform.c` and `src/game.c`).

- Build / run / debug (exact commands)
  - Standard cross-platform build:
    - cmake -S . -B build
    # AvatarQuest — AI assistant guide

    Purpose: orient an AI coding agent with the concrete, discoverable patterns in this repo so changes are low-risk and consistent.

    - Top-level facts
      - Language: C++ (sources are .cpp/.h). Build: CMake (>= 3.24). Uses SDL3 and optionally SDL_image/SDL_mixer when vendored.
      - Single executable target built from files under `src/` with public headers in `include/AvatarQuest/`.

    - Big-picture architecture (what to edit and why)
      - Entry: `src/main.cpp` wires the engine and creates the initial UI/game layer (factory in `include/AvatarQuest/AvatarQuest.h`).
      - Platform/renderer: `src/common/Window.*` and `src/common/Renderer.*` initialize SDL, create the window and SDL_Renderer, and expose `runMainLoop()`, `initRenderer()`, `getRenderer()`.
      - Game layer: `src/common/Game.*` — manages layers, converts SDL events to `Game::GameEvents`, and calls layer update/render each frame.
      - Example gameplay/UI: `src/AvatarQuest/AvatarQuestLayer.cpp` — shows asset loading (SDL_image), texture creation, input handling via `Game::getGameEvents()`.

    - Key project-specific patterns
      - Namespaces: `Game::`, `AvatarQuest::`. Classes often end with `Layer` for UI/game layers.
      - Engine containers: custom aliases like `Ref<>`, `Vector<>`, `UMap<>` appear in `src/common/*`; follow their usage rather than introducing STL unless consistent.
      - Event flow: SDL events are captured by platform code and mapped into `Game::GameEvents` (see `Game::handleEvent` in `src/common/Game.cpp`). Layers access events through `Game::getGameEvents()` in their `update()`.
      - Render flow: use `Renderer::beginRender()` -> issue draw commands (drawRect, drawFilledRect, drawThickLine, drawPrimitiveList) -> `Renderer::endRender()` (see `src/common/Renderer.cpp` for batching).
      - SDL resource lifecycle: SDL resources are manually created/destroyed (e.g., `SDL_CreateTextureFromSurface` / `SDL_DestroyTexture`). Match creations with corresponding destroy calls (see `AvatarQuestLayer::shutDown`).

    - Build / test / run (practical commands)
      - Configure and build:
        - cmake -S . -B build
        - cmake --build build
      - Windows + Visual Studio example (PowerShell):
        - cmake -S . -B build -G "Visual Studio 17 2022" -A x64
        - cmake --build build --config Debug
      - Vendor SDL and codecs (optional): add `-DAVATARQUEST_VENDOR_SDL3=ON` and optional toggles like `-DSDLIMAGE_AVIF=OFF`.
      - Enable tests: `-DAVATARQUEST_BUILD_TESTS=ON` (tests should live under `tests/` if added).
      - Run: execute the produced binary from the build output directory (e.g., `./build/AvatarQuest` or `build\Debug\AvatarQuest.exe`).

    - Files to inspect for concrete examples
      - `src/main.cpp` — engine wiring and initial layer creation.
      - `include/AvatarQuest/AvatarQuest.h` — public factory: `AvatarQuest::CreateAvatarQuestLayer()`.
      - `src/AvatarQuest/AvatarQuestLayer.cpp` — SDL_image usage (`IMG_LoadTyped_IO`), texture creation, and event handling example.
      - `src/common/Game.cpp` — event conversion and layer update/render loop.
      - `src/common/Renderer.cpp` — command batching, `beginRender` / `endRender`, and geometry helpers (e.g., `RenderThickLineAlt`).
      - `CMakeLists.txt` and `scripts/setup_env.sh` / `README.md` for build flags and vendoring notes.

    - How to add code safely (explicit checklist)
      - Put public headers in `include/AvatarQuest/` and sources in `src/YourModule/`.
      - Add new .cpp files to `CMakeLists.txt` for the `AvatarQuest` executable target.
      - Expose new gameplay modules via a factory in a public header when the engine should create them (follow `AvatarQuest::CreateAvatarQuestLayer`).
      - Keep SDL lifecycle and windowing in `Window`/`Renderer`; layers should use `Renderer` APIs and `Game::getGameEvents()`.

    If anything is unclear or you'd like a short how‑to (add a new layer, wire audio, vendor SDL_image choices), tell me which area and I'll expand this file with concrete code snippets and CMake edits.
