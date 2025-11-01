#include "Common.h"
#include "Sound.h"

#include <SDL3_mixer/SDL_mixer.h>
#include <mutex>
#include <atomic>
#include <memory>

namespace {
    // Consolidated sound state container to avoid scattered globals
    struct SoundState {
        bool mixerInited = false;
        MIX_Mixer* mixer = nullptr;
        MIX_Track* musicTrack = nullptr;

        Vector<Ref<Sound::Sfx>> sfx;
        Vector<Ref<Sound::Music>> music;

        // Tracks to destroy on main thread (queued from mixer thread)
        Vector<MIX_Track*> trashTracks;
        std::mutex trashMutex;

        // Category gains (attenuation only). 1.0 = full volume, 0.0 = silent.
        std::atomic<float> sfxGain{1.0f};    // multiplier for SFX
        std::atomic<float> musicGain{1.0f};  // multiplier for music
        float musicBaseGain = 1.0f;          // per-track base for current music

        struct ActiveSfxTrack {
            MIX_Track* track{};
            float baseGain{};
            std::unique_ptr<float> cbUser; // RAII for cooked-callback userdata

            ActiveSfxTrack() = default;
            ActiveSfxTrack(MIX_Track* t, float b, std::unique_ptr<float> u)
                : track(t), baseGain(b), cbUser(std::move(u)) {}
            ActiveSfxTrack(const ActiveSfxTrack&) = delete;
            ActiveSfxTrack& operator=(const ActiveSfxTrack&) = delete;
            ActiveSfxTrack(ActiveSfxTrack&&) noexcept = default;
            ActiveSfxTrack& operator=(ActiveSfxTrack&&) noexcept = default;
        };
        Vector<ActiveSfxTrack> activeSfxTracks;

        void* musicCbUser = nullptr; // kept for symmetry, currently unused in callback
    };

    inline SoundState& SS() {
        static SoundState s;
        return s;
    }

    // Per-track cooked callbacks to apply gain without using MIX_SetTrackGain
    void SDLCALL SfxCookedCB(void* userdata, MIX_Track* /*track*/, const SDL_AudioSpec* spec, float* pcm, int samples) {
        if (!userdata || !pcm || !spec || samples <= 0) return;
        const float base = *static_cast<const float*>(userdata);
        const float cat = SS().sfxGain.load(std::memory_order_relaxed);
        const float factor = base * cat;
        if (factor == 1.0f) return;
        const int total = samples * farmMax(1, spec->channels);
        for (int i = 0; i < total; ++i) { pcm[i] *= factor; }
    }

    void SDLCALL MusicCookedCB(void* userdata, MIX_Track* /*track*/, const SDL_AudioSpec* spec, float* pcm, int samples) {
        (void)userdata; // not used; we read state values instead
        if (!pcm || !spec || samples <= 0) return;
        const float cat = SS().musicGain.load(std::memory_order_relaxed);
        const float factor = SS().musicBaseGain * cat;
        if (factor == 1.0f) return;
        const int total = samples * farmMax(1, spec->channels);
        for (int i = 0; i < total; ++i) { pcm[i] *= factor; }
    }

    // Auto-destroy tracks when they finish mixing
    void SDLCALL OnTrackStopped(void* userdata, MIX_Track* track) {
        (void)userdata;
        if (!track) return;
        // Don't destroy from mixer thread; queue for main-thread destruction.
        auto& st = SS();
        std::scoped_lock<std::mutex> lock(st.trashMutex);
        st.trashTracks.push_back(track);
    }
}

bool Sound::init()
{
    auto& st = SS();
    if (st.mixerInited) {
        return true;
    }

    if (!MIX_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_mixer init failed: %s", SDL_GetError());
        return false;
    }

    st.mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!st.mixer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_CreateMixerDevice failed: %s", SDL_GetError());
        MIX_Quit();
        return false;
    }

    // Create a dedicated music track we can reuse
    st.musicTrack = MIX_CreateTrack(st.mixer);
    if (!st.musicTrack) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_CreateTrack (music) failed: %s", SDL_GetError());
        MIX_DestroyMixer(st.mixer);
        st.mixer = nullptr;
        MIX_Quit();
        return false;
    }

    st.mixerInited = true;
    return true;
}

void Sound::shutdown()
{
    auto& st = SS();
    if (!st.mixerInited) return;

    // Stop and destroy any remaining SFX tracks; RAII will free cooked userdata
    for (auto& a : st.activeSfxTracks) {
        if (a.track) {
            MIX_StopTrack(a.track, 0);
            MIX_DestroyTrack(a.track);
            a.track = nullptr;
        }
    }
    st.activeSfxTracks.clear();

    // Stop music and close
    if (st.musicTrack) {
        // stop immediately
        MIX_StopTrack(st.musicTrack, 0);
        MIX_DestroyTrack(st.musicTrack);
        st.musicTrack = nullptr;
    }

    // Free loaded audio
    for (auto& s : st.sfx) {
        if (s && s->handle) {
            MIX_DestroyAudio(s->handle);
            s->handle = nullptr;
        }
    }
    st.sfx.clear();

    for (auto& m : st.music) {
        if (m && m->handle) {
            MIX_DestroyAudio(m->handle);
            m->handle = nullptr;
        }
    }
    st.music.clear();

    if (st.mixer) {
        // cleanup music callback userdata if any
        if (st.musicCbUser) { delete static_cast<float*>(st.musicCbUser); st.musicCbUser = nullptr; }
        MIX_DestroyMixer(st.mixer);
        st.mixer = nullptr;
    }

    MIX_Quit();
    st.mixerInited = false;
}

void Sound::update()
{
    auto& st = SS();
    if (!st.mixerInited) return;
    // Drain and destroy any finished SFX tracks queued by the mixer thread.
    Vector<MIX_Track*> toDestroy;
    {
        std::scoped_lock<std::mutex> lock(st.trashMutex);
        toDestroy.swap(st.trashTracks);
    }
    for (auto* t : toDestroy) {
        if (t && t != st.musicTrack) {
            // remove from active SFX list if present
            for (auto it = st.activeSfxTracks.begin(); it != st.activeSfxTracks.end(); ++it) {
                if (it->track == t) {
                    st.activeSfxTracks.erase(it);
                    break;
                }
            }
            MIX_DestroyTrack(t);
        }
    }
}

Sound::Sfx* Sound::loadSfx(const std::string& path)
{
    auto& st = SS();
    if (!st.mixerInited || !st.mixer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Sound::loadSfx called before Sound::init");
        return nullptr;
    }

    MIX_Audio* audio = MIX_LoadAudio(st.mixer, path.c_str(), /*predecode*/ false);
    if (!audio) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_LoadAudio failed for '%s': %s", path.c_str(), SDL_GetError());
        return nullptr;
    }

    auto sfxRef = CreateRef<Sound::Sfx>();
    sfxRef->handle = audio;
    st.sfx.push_back(sfxRef);
    return sfxRef.get();
}

void Sound::unloadSfx(Sound::Sfx* s)
{
    if (!s) return;
    auto& st = SS();
    for (auto it = st.sfx.begin(); it != st.sfx.end(); ++it) {
        if (it->get() == s) {
            if ((*it)->handle) {
                MIX_DestroyAudio((*it)->handle);
                (*it)->handle = nullptr;
            }
            st.sfx.erase(it);
            break;
        }
    }
}

Sound::Music* Sound::loadMusic(const std::string& path)
{
    auto& st = SS();
    if (!st.mixerInited || !st.mixer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Sound::loadMusic called before Sound::init");
        return nullptr;
    }

    MIX_Audio* audio = MIX_LoadAudio(st.mixer, path.c_str(), /*predecode*/ false);
    if (!audio) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_LoadAudio failed for '%s': %s", path.c_str(), SDL_GetError());
        return nullptr;
    }

    auto musicRef = CreateRef<Sound::Music>();
    musicRef->handle = audio;
    st.music.push_back(musicRef);
    return musicRef.get();
}

void Sound::unloadMusic(Sound::Music* m)
{
    if (!m) return;
    auto& st = SS();
    for (auto it = st.music.begin(); it != st.music.end(); ++it) {
        if (it->get() == m) {
            if ((*it)->handle) {
                MIX_DestroyAudio((*it)->handle);
                (*it)->handle = nullptr;
            }
            st.music.erase(it);
            break;
        }
    }
}

bool Sound::playSfx(Sound::Sfx* s, int loops, int channel, int volume)
{
    auto& st = SS();
    (void)channel; // SDL_mixer 3.x doesn't have channel indices; we autogenerate tracks.
    // per-track volume (0..128) expressed as base gain [0..1]
    float baseGain = static_cast<float>(farmMax(0, farmMin(volume, 128))) / 128.0f;
    if (!st.mixerInited || !st.mixer || !s || !s->handle) return false;

    MIX_Track* track = MIX_CreateTrack(st.mixer);
    if (!track) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_CreateTrack failed: %s", SDL_GetError());
        return false;
    }

    if (!MIX_SetTrackAudio(track, s->handle)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_SetTrackAudio failed: %s", SDL_GetError());
        MIX_DestroyTrack(track);
        return false;
    }

    // Use cooked-callback to scale samples; stash baseGain in RAII pointer
    auto user = std::make_unique<float>(baseGain);
    MIX_SetTrackCookedCallback(track, SfxCookedCB, user.get());

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

    // Track this SFX so future setSfxVolume() can adjust it
    st.activeSfxTracks.emplace_back(track, baseGain, std::move(user));

    return true;
}

bool Sound::playMusic(Sound::Music* m, int loops, int volume)
{
    auto& st = SS();
    if (!st.mixerInited || !st.mixer || !st.musicTrack || !m || !m->handle) return false;

    if (!MIX_SetTrackAudio(st.musicTrack, m->handle)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_SetTrackAudio (music) failed: %s", SDL_GetError());
        return false;
    }

    // per-track base gain from requested volume; apply via cooked callback
    st.musicBaseGain = static_cast<float>(farmMax(0, farmMin(volume, 128))) / 128.0f;
    // free previous userdata if any
    if (st.musicCbUser) { delete static_cast<float*>(st.musicCbUser); st.musicCbUser = nullptr; }
    st.musicCbUser = nullptr; // not used by MusicCookedCB
    MIX_SetTrackCookedCallback(st.musicTrack, MusicCookedCB, st.musicCbUser);

    SDL_PropertiesID opts = SDL_CreateProperties();
    SDL_SetNumberProperty(opts, MIX_PROP_PLAY_LOOPS_NUMBER, static_cast<Sint64>(loops));
    bool ok = MIX_PlayTrack(st.musicTrack, opts);
    SDL_DestroyProperties(opts);
    if (!ok) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_PlayTrack (music) failed: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool Sound::playMusicFadeIn(Sound::Music* m, int loops, int volume, int fadeInMs)
{
    auto& st = SS();
    if (!st.mixerInited || !st.mixer || !st.musicTrack || !m || !m->handle) return false;

    if (!MIX_SetTrackAudio(st.musicTrack, m->handle)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_SetTrackAudio (music) failed: %s", SDL_GetError());
        return false;
    }

    // per-track base gain from requested volume; apply via cooked callback
    st.musicBaseGain = static_cast<float>(farmMax(0, farmMin(volume, 128))) / 128.0f;
    if (st.musicCbUser) { delete static_cast<float*>(st.musicCbUser); st.musicCbUser = nullptr; }
    st.musicCbUser = nullptr;
    MIX_SetTrackCookedCallback(st.musicTrack, MusicCookedCB, st.musicCbUser);

    SDL_PropertiesID opts = SDL_CreateProperties();
    SDL_SetNumberProperty(opts, MIX_PROP_PLAY_LOOPS_NUMBER, static_cast<Sint64>(loops));
    if (fadeInMs > 0) {
        SDL_SetNumberProperty(opts, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, static_cast<Sint64>(fadeInMs));
    }
    bool ok = MIX_PlayTrack(st.musicTrack, opts);
    SDL_DestroyProperties(opts);
    if (!ok) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_PlayTrack (music fade-in) failed: %s", SDL_GetError());
        return false;
    }
    return true;
}

void Sound::stopMusic(int fadeMs)
{
    auto& st = SS();
    if (!st.mixerInited || !st.musicTrack) return;

    Sint64 frames = 0;
    if (fadeMs > 0) {
        frames = MIX_TrackMSToFrames(st.musicTrack, static_cast<Sint64>(fadeMs));
        if (frames < 0) frames = 0; // fallback
    }
    MIX_StopTrack(st.musicTrack, frames);
}

void Sound::setMasterVolume(int volume)
{
    auto& st = SS();
    if (!st.mixerInited || !st.mixer) return;
    float gain = static_cast<float>(farmMax(0, farmMin(volume, 128))) / 128.0f;
    MIX_SetMasterGain(st.mixer, gain);
}

void Sound::setMusicVolume(int volume)
{
    auto& st = SS();
    if (!st.mixerInited) return;
    float g = static_cast<float>(farmMax(0, farmMin(volume, 128))) / 128.0f;
    if (g > 1.0f) g = 1.0f; // attenuation only
    st.musicGain = g;
    // No mixer-side gain changes; cooked callback scales samples using musicBaseGain * musicGain
}

void Sound::setSfxVolume(int volume)
{
    auto& st = SS();
    if (!st.mixerInited) return;
    float g = static_cast<float>(farmMax(0, farmMin(volume, 128))) / 128.0f;
    if (g > 1.0f) g = 1.0f; // attenuation only
    st.sfxGain = g;
    // No per-track gain updates needed; cooked callback reads sfxGain atomically at mix time
}
