#pragma once
#include <SDL3/SDL.h>
#include <stdint.h>
#include <string.h>

// Detect if the real API exists (SDL 3.4+). If yes, don't redefine it.
#ifndef SDL_VERSION_ATLEAST
#define SDL_VERSION_ATLEAST(X,Y,Z) 0
#endif

#if !SDL_VERSION_ATLEAST(3,4,0)

/* Compat replacement for:
   SDL_PutAudioStreamPlanarData(SDL_AudioStream* stream,
                                const void* const* planes,
                                int channels, int frames)

   Behavior:
   - If channels < 0, infer channel count and sample format from the stream's *source* spec.
   - Interleave 'frames' samples from each channel plane (or zero if plane is NULL).
   - Push interleaved PCM to SDL via SDL_PutAudioStreamData().
   - Returns SDL_TRUE on success, SDL_FALSE on failure.
*/
static bool SDL_PutAudioStreamPlanarData(SDL_AudioStream *stream,
                                             const void * const *planes,
                                             int channels,
                                             int frames)
{
    if (!stream || !planes || frames <= 0) {
        return false;
    }

    SDL_AudioSpec srcspec, dstspec;
    if (!SDL_GetAudioStreamFormat(stream, &srcspec, &dstspec)) {
        // If we can't query, bail—mixer normally sets this at creation time.
        return false;
    }

    int ch = channels;
    if (ch <= 0) {
        ch = (int)srcspec.channels;   // handle -1 or 0 => use stream's src channels
    }
    if (ch <= 0) {
        return false;
    }

    // bytes-per-sample from source format
    const int bps = (int)(SDL_AUDIO_BITSIZE(srcspec.format) / 8);
    if (bps <= 0) {
        return false;
    }

    // total interleaved buffer size
    const size_t bytes_per_frame = (size_t)bps * (size_t)ch;
    const size_t total_bytes = bytes_per_frame * (size_t)frames;

    uint8_t *interleaved = (uint8_t *)SDL_malloc(total_bytes);
    if (!interleaved) {
        return false;
    }

    // Interleave: for each frame, write samples from channel 0..ch-1
    for (int f = 0; f < frames; ++f) {
        uint8_t *out = interleaved + ((size_t)f * bytes_per_frame);
        for (int c = 0; c < ch; ++c) {
            const uint8_t *src = NULL;
            if (planes[c]) {
                src = ((const uint8_t *)planes[c]) + ((size_t)f * (size_t)bps);
                memcpy(out + (size_t)c * (size_t)bps, src, (size_t)bps);
            } else {
                // NULL plane => silence for this channel (needed by decoder_voc.c)
                memset(out + (size_t)c * (size_t)bps, 0, (size_t)bps);
            }
        }
    }

    bool ok = (bool)SDL_PutAudioStreamData(stream, interleaved, (int)total_bytes);
    SDL_free(interleaved);
    return ok;
}

typedef void(SDLCALL *SDL_AudioStreamDataCompleteCallback)(void *userdata, const void *buf, int buflen);

// Compat shim: falls back to a copy + immediate completion.
static inline bool SDL_PutAudioStreamDataNoCopy(SDL_AudioStream *stream,
                                                const void *buf, int len,
                                                SDL_AudioStreamDataCompleteCallback cb,
                                                void *userdata)
{
    if (!stream || !buf || len <= 0)
        return false;

    // This copies the data into SDL's internal queue; once it returns true,
    // the caller may safely reuse/free 'buf'.
    if (!SDL_PutAudioStreamData(stream, buf, len)) {
        return false;
    }

    // Emulate "NoCopy completion": since we *did* copy, the external buffer
    // is no longer needed—fire the completion right now.
    if (cb) {
        // SDL guarantees the real callback may run from any thread; this
        // compat calls it synchronously on the caller's thread.
        cb(userdata, buf, len);
    }
    return true;
}


#endif
