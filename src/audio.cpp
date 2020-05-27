#include "audio.h"

#include "util/log.h"
#include "math/smek_math.h"

namespace Audio {

void audio_callback(AudioStruct *audio_struct, f32 *stream, int len) {
    const u32 SAMPLES = len / sizeof(f32);
    const f32 TIME_STEP = 1.0f / audio_struct->sample_rate;

    for (u32 i = 0; i < SAMPLES; i++) {
        stream[i] = 0.0;
    }

    for (u32 source_id = 0; source_id < NUM_SOURCES; source_id++) {
        SoundSource *source = audio_struct->sources + source_id;
        if (!source->active) continue;

        u64 index;
        Asset::Sound *sound = Asset::fetch_sound(source->asset_id);
        for (u32 i = 0; i < SAMPLES; i += 2) {
            source->sample += sound->sample_rate * TIME_STEP;
            index = source->sample;

            if (index * sound->channels >= sound->num_samples) {
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

            if (sound->channels == 2) {
                left = sound->data[index * 2 + 0];
                right = sound->data[index * 2 + 1];
            } else {
                // mono
                f32 sample;
                sample = sound->data[index];
                left = sample;
                right = sample;
            }

            stream[i+0] = left;
            stream[i+1] = right;
        }
    }
}

} // namespace Audio

void audio_callback(Audio::AudioStruct *audio_struct, f32 *stream, int len) {
    Audio::audio_callback(audio_struct, stream, len);
}