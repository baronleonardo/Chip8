#ifndef RENDERER_H
#define RENDERER_H

#include "utils/type_alias.h"
// #include "result.h"

#include <stdbool.h>

typedef struct
{
    i32 width;
    i32 height;
    i32 scale;
    u8* display;
    void* window;
    void* sdl_renderer;
    // bool is_running;
    bool valid;
} Renderer;

Renderer
Renderer_init(i32 scale);

void
Renderer_render(Renderer* self);

bool
Renderer_toggle_pixel(Renderer* self, u32 pos_x, u32 pos_y);

void
Renderer_clear(Renderer* self);

void
Renderer_deinit(Renderer* self);

#endif // RENDERER_H