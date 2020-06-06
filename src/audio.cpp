#include "audio.h"

#include "util/log.h"
#include "math/smek_math.h"

namespace Audio {

void audio_callback(AudioStruct *audio_struct, f32 *stream, int len) {
    audio_struct->lock();
    defer { audio_struct->unlock(); };
    const u32 SAMPLES = len / sizeof(f32);
    const f32 TIME_STEP = 1.0f / audio_struct->sample_rate;

    for (u32 i = 0; i < SAMPLES; i++) {
        stream[i] = 0.0;
    }

    for (u32 source_id = 0; source_id < NUM_SOURCES; source_id++) {
        SoundSource *source = audio_struct->sources + source_id;
        if (!source->active) continue;

        u64 index;
        for (u32 i = 0; i < SAMPLES; i += 2) {
            source->sample += source->sample_rate * TIME_STEP;
            index = source->sample;

            if (index * source->channels >= source->num_samples) {
                if (source->repeat) {
                    source->sample = 0;
                    index = 0;
                } else {
                    source->active = false;
                    break;
                }
            }

            f32 left;
            f32 right;

            if (source->channels == 2) {
                left = source->data[index * 2 + 0];
                right = source->data[index * 2 + 1];
            } else {
                // mono
                f32 sample;
                sample = source->data[index];
                left = sample;
                right = sample;
            }

            stream[i+0] += left * source->gain;
            stream[i+1] += right * source->gain;
        }
    }
}

AudioID AudioStruct::play_sound(AssetID asset_id, SoundSourceSettings source_settings) {
    lock();
    defer { unlock(); };
    for (u32 source_id = 0; source_id < NUM_SOURCES; source_id++) {
        SoundSource *source = sources + source_id;
        if (source->active) {
            continue;
        }
        Asset::Sound *sound = Asset::fetch_sound(asset_id);
        AudioID id = { .gen = ++source->gen, .slot = source_id };
        *source = (SoundSource) {
            .channels = sound->channels,
            .sample_rate = sound->sample_rate,
            .num_samples = sound->num_samples,
            .data = sound->data,
            .sample = 0.0,
            .gain = source_settings.gain,
            .active = true,
            .repeat = source_settings.repeat,
            .gen = source->gen,
        };
        return id;
    }
    ERROR("No free sources, skipping sound");
    return { .gen = 0, .slot = NUM_SOURCES };
}

void AudioStruct::stop_sound(AudioID id) {
    if (id.gen == 0) {
        WARN("Tried to stop AudioID that was never started");
        return;
    }
    ASSERT(id.slot < NUM_SOURCES, "Tried to stop invalid AudioID");
    lock();
    defer { unlock(); };
    SoundSource *source = sources + id.slot;
    if (id.gen != source->gen) {
        WARN("Tried to stop outdated AudioID");
        return;
    }
    source->active = false;
    return;
}

void AudioStruct::stop_all() {
    lock();
    defer { unlock(); };
    for (u32 source_id = 0; source_id < NUM_SOURCES; source_id++) {
        SoundSource *source = sources + source_id;
        source->active = false;
    }
}

} // namespace Audio

void audio_callback(Audio::AudioStruct *audio_struct, f32 *stream, int len) {
    Audio::audio_callback(audio_struct, stream, len);
}
