#pragma once
#include "SDL2/SDL.h"

#include "asset/asset.h"
#include "util/log.h"

namespace Audio {

const u32 SAMPLE_RATE = 48000;
const u32 NUM_SOURCES = 10;

struct SoundSource {
    AssetID asset_id;
    f32 sample;
    bool active;
    bool repeat;
};

struct AudioStruct {
    bool active = false;
    SDL_AudioDeviceID dev;
    SoundSource sources[NUM_SOURCES];

    void lock() {
        if (active)
            SDL_LockAudioDevice(dev);
    }
    void unlock() {
        if (active)
            SDL_UnlockAudioDevice(dev);
    }
};

void audio_callback(AudioStruct *audio_struct, u8 *stream, int len);

} // namespace Audio

extern "C" void audio_callback(Audio::AudioStruct *audio_struct, u8 *stream, int len);
typedef void(*AudioCallbackFunc)(void *, u8 *, int);
