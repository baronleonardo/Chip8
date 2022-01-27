#include "speaker.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include <math.h>
#include <stdint.h>

// This callback will play a square wave
static void
Speaker__callback__(void* userdata, u8* stream, int len);

Speaker
Speaker_init()
{
    Speaker speaker = {};

    if(SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        fputs(SDL_GetError(), stderr);
        SDL_ClearError();
        speaker.valid = false;
        return speaker;
    }

    /* a general specification */
    speaker.specs.freq = 44100;
    speaker.specs.format = AUDIO_S16;
    speaker.specs.channels = 1; /* 1, 2, 4, or 6 */
    speaker.specs.samples = 2048; /* power of 2, or 0 and env SDL_AUDIO_SAMPLES is used */
    speaker.specs.callback = Speaker__callback__; /* can not be NULL */

    speaker.valid = true;
    speaker.sample_pos = 0;
    speaker.freq = AUDIO_FREQ;
    speaker.dev_id = 0;
    speaker.amplitude = AUDIO_AMPLITUDE;
    speaker.is_playing = false;

    return speaker;
}

void
Speaker_play(Speaker* self, f64 freq, i32 amplitude)
{
    if(!self || !self->valid)
    {
        return;
    }

    if(freq == 0) freq = AUDIO_FREQ;
    if(amplitude < 0) amplitude = AUDIO_AMPLITUDE;

    SDL_AudioSpec have;

    self->freq = freq;
    self->amplitude = amplitude;
    self->specs.userdata = self;

    if(self->dev_id == 0)
    {
        self->dev_id = SDL_OpenAudioDevice(NULL, 0, &self->specs, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
    }

    if(self->dev_id == 0)
    {
        fputs(SDL_GetError(), stderr);
        self->valid = false;
    }

    self->is_playing = true;
    SDL_PauseAudioDevice(self->dev_id, 0); /* play! */
}

void
Speaker_stop(Speaker* self)
{
    if(!self || !self->valid)
    {
        return;
    }

    if(!self->is_playing)
    {
        return;
    }

    if(self->dev_id == 0)
    {
        fputs("Error: Audio: cannot stop a null device\n", stderr);
        return;
    }

    self->is_playing = false;
    SDL_PauseAudioDevice(self->dev_id, 1); /* stop! */
}

void
Speaker_deinit(Speaker* self)
{
    if(!self)
    {
        return;
    }

    if(self->dev_id != 0)
    {
        SDL_CloseAudioDevice(self->dev_id);
    }
}

void
Speaker__callback__(void* userdata, u8* stream, int len)
{
    Speaker *speaker = (Speaker *)userdata;

    i16 *buffer = (i16*)stream;
    len = len / 2; // 2 bytes per sample for AUDIO_S16SYS

    for(int iii = 0; iii < len; iii++, speaker->sample_pos++)
    {
        double time = (f64)speaker->sample_pos / (f64)speaker->specs.freq;
        ((i16*)stream)[iii] = (i16)(speaker->amplitude * sin(2.0f * M_PI * speaker->freq * time));
    }
}