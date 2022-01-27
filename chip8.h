#ifndef CHIP8_H
#define CHIP8_H

#include "utils/type_alias.h"

#include <stdbool.h>

#include "cpu.h"
#include "keyboard.h"
#include "speaker.h"
#include "renderer.h"
#include "utils/string.h"

typedef struct {
    u8 fps;
    u32 fps_interval;
    u64 start_time;
    u64 past_time;
    u64 now;
    bool valid;
    bool is_running;
    Cpu cpu;
    Keyboard* keyboard;
    Renderer* renderer;
    Speaker* speaker;
} Chip8;

Chip8
Chip8_init(String rom_path, u8 screen_scale, u8 speed);

void
Chip8_mainloop(Chip8* self);

void
Chip8_deinit(Chip8* self);

#endif // CHIP8_H