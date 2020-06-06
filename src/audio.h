#pragma once
#include "SDL2/SDL.h"

#include "asset/asset.h"
#include "util/log.h"

///# Audio
//

struct AudioID {
    u32 gen;
    u32 slot;
};

namespace Audio {

const u32 NUM_SOURCES = 10;

struct SoundSource {
    u32 channels;
    u32 sample_rate;
    u32 num_samples;
    f32 *data;
    f32 sample;
    f32 gain;
    bool active;
    bool repeat;

    u32 gen;
};

//// Sound source settings
// This struct is used when calling <code>play_sound</code> so you can
// specify only the variables you want to be different from their
// default values, without having to specify all variables before the
// one you want different (as is the case for default parameters to
// functions). See <code>play_sound</code> for usage.
struct SoundSourceSettings {
    f32 gain = 0.33;
    bool repeat = false;
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

    AudioID play_sound(AssetID asset_id, SoundSourceSettings source_settings = {});
    void stop_sound(AudioID id);

    void stop_all();
};

void audio_callback(AudioStruct *audio_struct, f32 *stream, int len);

} // namespace Audio

extern "C" void audio_callback(Audio::AudioStruct *audio_struct, f32 *stream, int len);
typedef void(*AudioCallbackFunc)(void *, f32 *, int);

#if 0

//// Playing a sound
// Play a sound using the sound system.
AudioID AudioStruct::play_sound(AssetID asset_id, SoundSourceSettings source_settings = {});
//
// Use all default values:
play_sound(asset_id);
//
// Change only the gain:
play_sound(asset_id, { .gain=0.3 });
//
// Set only repeat:
play_sound(asset_id, { .repeat=true });
//
// And so on for other settings.

///*
// Stop a currently playing sound.
void stop_sound(AudioID id);

///*
// Stop all currently playing sounds.
void stop_all();

#endif
