#pragma once
#include "SDL2/SDL.h"

#include "asset/asset.h"

namespace Audio {

const u32 SAMPLE_RATE = 48000;

struct AudioStruct {
    SDL_AudioDeviceID dev;
    u64 steps;

    void lock() {
        SDL_LockAudioDevice(dev);
    }
    void unlock() {
        SDL_UnlockAudioDevice(dev);
    }
};

void audio_callback(AudioStruct *audio_struct, u8 *stream, int len);

} // namespace Audio

extern "C" void audio_callback(Audio::AudioStruct *audio_struct, u8 *stream, int len);
typedef void(*AudioCallbackFunc)(Audio::AudioStruct *, u8 *, int);
