#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdbool.h>

#include "utils/type_alias.h"

#include <SDL2/SDL.h>

#define AUDIO_AMPLITUDE     INT16_MAX
#define AUDIO_FREQ          441.0

typedef struct {
    bool valid;
    u16 dev_id;
    SDL_AudioSpec specs;
    u32 sample_pos;
    f64 freq;
    i32 amplitude;
    bool is_playing;
} Speaker;

Speaker
Speaker_init();


/// @param: freq: if zero, it will be AUDIO_FREQ
/// @param: amplitude: if it less than 0, it will be AUDIO_AMPLITUDE
void
Speaker_play(Speaker* self, f64 freq, i32 amplitude);

void
Speaker_stop(Speaker* self);

void
Speaker_deinit(Speaker* self);

#endif // SPEAKER_H