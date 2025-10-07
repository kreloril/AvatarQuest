# AvatarQuest Architecture Overview

AvatarQuest separates platform responsibilities from gameplay code to keep the core systems modular and testable.

## Modules

### Platform Layer (`src/platform.c`)
* Initializes SDL subsystems.
* Creates the primary window and renderer.
* Owns the main loop and distributes events, timing, and rendering control to the gameplay layer.

### Gameplay Layer (`src/game.c`)
* Manages the `AvatarQuestGame` state container.
* Handles input through SDL events forwarded by the platform layer.
* Updates game logic each frame and issues draw calls using the provided renderer.

### Entry Point (`src/main.c`)
* Bootstraps the platform layer and creates the gameplay state.
* Delegates to the platform main loop and performs shutdown cleanup.

## Future Extensions

* Introduce a resource manager for loading textures, audio, and configuration files.
* Add scene management to swap between menus, gameplay, and cutscenes.
* Integrate scripting or data-driven entity definitions to accelerate iteration.
