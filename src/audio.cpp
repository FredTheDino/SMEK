#include "audio.h"

#include "util/log.h"
#include "math/smek_math.h"
#include "entity/entity.h"
#include "event.h"
#include "game.h"

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
        AudioID audio_id = {
            .gen = ++source->gen,
            .slot = source_id,
        };
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
        SoundEntity sound_entity = {};
        sound_entity.asset_id = asset_id;
        sound_entity.sound_source_settings = source_settings;
        sound_entity.audio_id = audio_id;
        EntityID entity_id = GAMESTATE()->entity_system.add(sound_entity);
        return audio_id;
    }
    ERROR("No free sources, skipping sound");
    return { .gen = 0, .slot = NUM_SOURCES };
}

void AudioStruct::stop_sound(AudioID id) {
    if (id.gen == 0) {
        WARN("Tried to stop AudioID that was never started");
        return;
    }
    if (id.slot >= NUM_SOURCES) {
        WARN("Tried to stop invalid AudioID");
        return;
    }
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

bool AudioStruct::is_playing(AudioID id) {
    CHECK(id.slot < NUM_SOURCES, "Tried to access invalid AudioID");
    SoundSource source = sources[id.slot];
    CHECK(id.gen <= source.gen, "Tried to access AudioID from the future?");
    return id.gen == source.gen && source.active;
}

} // namespace Audio

void EventSystem::EventCreateSoundEntity::callback() {
    GAMESTATE()->audio_struct->play_sound(AssetID(asset_id_hash), { .gain = gain, .repeat = repeat });
}

void audio_callback(Audio::AudioStruct *audio_struct, f32 *stream, int len) {
    Audio::audio_callback(audio_struct, stream, len);
}
