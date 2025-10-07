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
    - cmake --build build
  - On Windows with Visual Studio generators, choose generator and architecture when needed (example):
    - cmake -S . -B build -G "Visual Studio 17 2022" -A x64
    - cmake --build build --config Debug
  - Enable tests: pass the cache option `-DAVATARQUEST_BUILD_TESTS=ON` to `cmake` (the CMakeLists declares this option but tests are currently OFF by default).
  - Run: execute the produced binary from the repo root (paths vary by generator): `./build/AvatarQuest` or `build\Debug\AvatarQuest.exe` on MSVC Debug.
  - The repository includes `scripts/setup_env.sh` that runs a thin CMake invocation; on Windows use the equivalent PowerShell commands rather than executing the bash script.

- SDL3 / dependency notes
  - CMake expects an SDL3 config package. On many systems this means SDL3 was installed with a CMake config file. On Windows developers commonly use vcpkg or install SDL3 and make its `SDL3Config.cmake` discoverable (pass `-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake` or set CMAKE_PREFIX_PATH).
  - Target link: `target_link_libraries(AvatarQuest PRIVATE SDL3::SDL3)` — if you add libraries, link them similarly in `CMakeLists.txt`.

- How to add code safely (practical checklist)
  - New public headers go in `include/AvatarQuest/` and source in `src/`.
  - Update `CMakeLists.txt` to add new `src/*.c` files to the `add_executable(AvatarQuest ... )` list and keep `target_include_directories` as-is.
  - Follow existing naming conventions (`avatarquest_` prefix) and defensive null checks.
  - Keep platform-specific APIs (SDL calls, windowing) inside `src/platform.c` unless implementing cross-platform gameplay features.

- Integration and extension points
  - `avatarquest_platform_*` functions: platform lifecycle (initialize / shutdown / mainloop).
  - `avatarquest_game_*` functions: create/destroy/update/render/event attachment — extend game logic here or add modular subsystems that the game owns.
  - CMake option `AVATARQUEST_BUILD_TESTS` — place any unit tests under a `tests/` directory and conditionally add them to the build when this option is ON.

- Quick examples (where to look)
  - main flow: `src/main.c`
  - platform lifecycle + main loop: `src/platform.c` (see `g_window`, `g_renderer`, VSync call and `SDL_Delay(1)`).
  - gameplay skeleton: `src/game.c` (see `avatarquest_game_render()` for the simple hero square example).
  - build glue: `CMakeLists.txt`
  - architecture rationale: `docs/architecture.md` and `README.md`

If any section is unclear or you'd like guidance tailored to a specific change (new module, adding audio, or platform port), tell me which area and I will expand or iterate the instructions.
