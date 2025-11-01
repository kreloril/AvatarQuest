#pragma once

#include <string>
#include <cstdint>

// Forward declarations to avoid exposing SDL_mixer headers to all includers (SDL_mixer 3.x)
struct MIX_Audio;
struct MIX_Mixer;
struct MIX_Track;

namespace Sound {
    // Initialize/Shutdown SDL_mixer system
    bool init();
    // Pump cleanup and any pending audio maintenance. Call once per frame.
    void update();
    void shutdown();

    // Simple handles
    struct Sfx { MIX_Audio* handle = nullptr; };
    struct Music { MIX_Audio* handle = nullptr; };

    // Load/unload
    Sfx* loadSfx(const std::string& path);
    void unloadSfx(Sfx* s);

    Music* loadMusic(const std::string& path);
    void unloadMusic(Music* m);

    // Playback
    // channel: -1 picks first free channel
    bool playSfx(Sfx* s, int loops = 0, int channel = -1, int volume = 128);
    bool playMusic(Music* m, int loops = -1, int volume = 96);
    // Convenience: play music with a fade-in over the specified milliseconds
    bool playMusicFadeIn(Music* m, int loops = -1, int volume = 96, int fadeInMs = 250);
    void stopMusic(int fadeMs = 0);

    // Global volume (0-128 typical)
    void setMasterVolume(int volume);

    // Category volumes (0-128), applied as attenuation only (never above 1.0)
    // - Music volume applies immediately to the persistent music track
    // - SFX volume applies to currently playing SFX tracks and new ones
    void setMusicVolume(int volume);
    void setSfxVolume(int volume);
}
