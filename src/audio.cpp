#include "audio.h"

#include "math/smek_math.h"

namespace Audio {

void audio_callback(AudioStruct *audio_struct, u8 *stream, int len) {
    const u32 SAMPLES = len / sizeof(f32);

    f32 *output = (f32 *) stream; //TODO(gu)

    const f32 TIME_STEP = 1.0f / SAMPLE_RATE;

    for (u32 i = 0; i < SAMPLES; i += 2) {
        output[i+0] = Math::sin(TIME_STEP * (i + audio_struct->steps) * 440) * 0.3;
        output[i+1] = Math::sin(TIME_STEP * (i + audio_struct->steps) * 440) * 0.3;
    }

    audio_struct->steps += SAMPLES;
}

} // namespace Audio

void audio_callback(Audio::AudioStruct *audio_struct, u8 *stream, int len) {
    Audio::audio_callback(audio_struct, stream, len);
}
