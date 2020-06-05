#pragma once
#include "SDL2/SDL.h"

#include "asset/asset.h"
#include "util/log.h"

struct AudioID {
    u32 gen;
    u32 slot;
};

namespace Audio {

const u32 NUM_SOURCES = 10;

struct SoundSource {
    AssetID asset_id;
    f32 sample;
    bool active;
    bool repeat;

    u32 gen;
};

struct AudioStruct {
    bool active = false;
    u32 sample_rate;
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

    AudioID play_sound(AssetID asset_id, bool repeat=false);
    void stop_sound(AudioID id);
};

void audio_callback(AudioStruct *audio_struct, f32 *stream, int len);

} // namespace Audio

extern "C" void audio_callback(Audio::AudioStruct *audio_struct, f32 *stream, int len);
typedef void(*AudioCallbackFunc)(void *, f32 *, int);
