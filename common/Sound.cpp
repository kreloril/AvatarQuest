#include "Common.h"
#include "Sound.h"

#include <SDL3_mixer/SDL_mixer.h>
#include <mutex>

namespace {
    bool g_mixerInited = false;
    MIX_Mixer* g_mixer = nullptr;
    MIX_Track* g_musicTrack = nullptr;
    Vector<Ref<Sound::Sfx>> g_sfx;
    Vector<Ref<Sound::Music>> g_music;
    Vector<MIX_Track*> g_trashTracks; // tracks to destroy on main thread
    std::mutex g_trashMutex;

    // Auto-destroy tracks when they finish mixing
    void SDLCALL OnTrackStopped(void* userdata, MIX_Track* track) {
        (void)userdata;
        if (!track) return;
        // Don't destroy from mixer thread; queue for main-thread destruction.
        std::scoped_lock<std::mutex> lock(g_trashMutex);
        g_trashTracks.push_back(track);
    }
}

bool Sound::init()
{
    if (g_mixerInited) {
        return true;
    }

    if (!MIX_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_mixer init failed: %s", SDL_GetError());
        return false;
    }

    g_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!g_mixer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_CreateMixerDevice failed: %s", SDL_GetError());
        MIX_Quit();
        return false;
    }

    // Create a dedicated music track we can reuse
    g_musicTrack = MIX_CreateTrack(g_mixer);
    if (!g_musicTrack) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_CreateTrack (music) failed: %s", SDL_GetError());
        MIX_DestroyMixer(g_mixer);
        g_mixer = nullptr;
        MIX_Quit();
        return false;
    }

    g_mixerInited = true;
    return true;
}

void Sound::shutdown()
{
    if (!g_mixerInited) return;

    // Stop music and close
    if (g_musicTrack) {
        // stop immediately
        MIX_StopTrack(g_musicTrack, 0);
        MIX_DestroyTrack(g_musicTrack);
        g_musicTrack = nullptr;
    }

    // Free loaded audio
    for (auto& s : g_sfx) {
        if (s && s->handle) {
            MIX_DestroyAudio(s->handle);
            s->handle = nullptr;
        }
    }
    g_sfx.clear();

    for (auto& m : g_music) {
        if (m && m->handle) {
            MIX_DestroyAudio(m->handle);
            m->handle = nullptr;
        }
    }
    g_music.clear();

    if (g_mixer) {
        MIX_DestroyMixer(g_mixer);
        g_mixer = nullptr;
    }

    MIX_Quit();
    g_mixerInited = false;
}

void Sound::update()
{
    if (!g_mixerInited) return;
    // Drain and destroy any finished SFX tracks queued by the mixer thread.
    Vector<MIX_Track*> toDestroy;
    {
        std::scoped_lock<std::mutex> lock(g_trashMutex);
        toDestroy.swap(g_trashTracks);
    }
    for (auto* t : toDestroy) {
        if (t && t != g_musicTrack) {
            MIX_DestroyTrack(t);
        }
    }
}

Sound::Sfx* Sound::loadSfx(const std::string& path)
{
    if (!g_mixerInited || !g_mixer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Sound::loadSfx called before Sound::init");
        return nullptr;
    }

    MIX_Audio* audio = MIX_LoadAudio(g_mixer, path.c_str(), /*predecode*/ false);
    if (!audio) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_LoadAudio failed for '%s': %s", path.c_str(), SDL_GetError());
        return nullptr;
    }

    auto sfxRef = CreateRef<Sound::Sfx>();
    sfxRef->handle = audio;
    g_sfx.push_back(sfxRef);
    return sfxRef.get();
}

void Sound::unloadSfx(Sound::Sfx* s)
{
    if (!s) return;
    for (auto it = g_sfx.begin(); it != g_sfx.end(); ++it) {
        if (it->get() == s) {
            if ((*it)->handle) {
                MIX_DestroyAudio((*it)->handle);
                (*it)->handle = nullptr;
            }
            g_sfx.erase(it);
            break;
        }
    }
}

Sound::Music* Sound::loadMusic(const std::string& path)
{
    if (!g_mixerInited || !g_mixer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Sound::loadMusic called before Sound::init");
        return nullptr;
    }

    MIX_Audio* audio = MIX_LoadAudio(g_mixer, path.c_str(), /*predecode*/ false);
    if (!audio) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_LoadAudio failed for '%s': %s", path.c_str(), SDL_GetError());
        return nullptr;
    }

    auto musicRef = CreateRef<Sound::Music>();
    musicRef->handle = audio;
    g_music.push_back(musicRef);
    return musicRef.get();
}

void Sound::unloadMusic(Sound::Music* m)
{
    if (!m) return;
    for (auto it = g_music.begin(); it != g_music.end(); ++it) {
        if (it->get() == m) {
            if ((*it)->handle) {
                MIX_DestroyAudio((*it)->handle);
                (*it)->handle = nullptr;
            }
            g_music.erase(it);
            break;
        }
    }
}

bool Sound::playSfx(Sound::Sfx* s, int loops, int channel, int volume)
{
    (void)channel; // SDL_mixer 3.x doesn't have channel indices; we autogenerate tracks.
    if (!g_mixerInited || !g_mixer || !s || !s->handle) return false;

    MIX_Track* track = MIX_CreateTrack(g_mixer);
    if (!track) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_CreateTrack failed: %s", SDL_GetError());
        return false;
    }

    if (!MIX_SetTrackAudio(track, s->handle)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_SetTrackAudio failed: %s", SDL_GetError());
        MIX_DestroyTrack(track);
        return false;
    }

    // Set volume (gain)
    float gain = static_cast<float>(farmMax(0, farmMin(volume, 128))) / 128.0f;
    MIX_SetTrackGain(track, gain);

    // Auto-destroy when finished
    MIX_SetTrackStoppedCallback(track, OnTrackStopped, nullptr);

    SDL_PropertiesID opts = SDL_CreateProperties();
    if (loops != 0) {
        // loops: 0 = no loop; 1 = loop once; -1 = infinite
        SDL_SetNumberProperty(opts, MIX_PROP_PLAY_LOOPS_NUMBER, static_cast<Sint64>(loops));
    }

    bool ok = MIX_PlayTrack(track, opts);
    SDL_DestroyProperties(opts);
    if (!ok) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_PlayTrack failed: %s", SDL_GetError());
        // If play failed, destroy the track now to avoid a leak
        MIX_SetTrackStoppedCallback(track, nullptr, nullptr);
        MIX_DestroyTrack(track);
        return false;
    }

    return true;
}

bool Sound::playMusic(Sound::Music* m, int loops, int volume)
{
    if (!g_mixerInited || !g_mixer || !g_musicTrack || !m || !m->handle) return false;

    if (!MIX_SetTrackAudio(g_musicTrack, m->handle)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_SetTrackAudio (music) failed: %s", SDL_GetError());
        return false;
    }

    float gain = static_cast<float>(farmMax(0, farmMin(volume, 128))) / 128.0f;
    MIX_SetTrackGain(g_musicTrack, gain);

    SDL_PropertiesID opts = SDL_CreateProperties();
    SDL_SetNumberProperty(opts, MIX_PROP_PLAY_LOOPS_NUMBER, static_cast<Sint64>(loops));
    bool ok = MIX_PlayTrack(g_musicTrack, opts);
    SDL_DestroyProperties(opts);
    if (!ok) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_PlayTrack (music) failed: %s", SDL_GetError());
        return false;
    }
    return true;
}

void Sound::stopMusic(int fadeMs)
{
    if (!g_mixerInited || !g_musicTrack) return;

    Sint64 frames = 0;
    if (fadeMs > 0) {
        frames = MIX_TrackMSToFrames(g_musicTrack, static_cast<Sint64>(fadeMs));
        if (frames < 0) frames = 0; // fallback
    }
    MIX_StopTrack(g_musicTrack, frames);
}

void Sound::setMasterVolume(int volume)
{
    if (!g_mixerInited || !g_mixer) return;
    float gain = static_cast<float>(farmMax(0, farmMin(volume, 128))) / 128.0f;
    MIX_SetMasterGain(g_mixer, gain);
}
