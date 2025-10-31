## AvatarQuest — AI assistant guide

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
