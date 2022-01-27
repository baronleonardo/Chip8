#include "renderer.h"
#include "error.h"

#include <stdbool.h>
#include <math.h>
#include <sys/time.h>

#include <SDL2/SDL.h>

#define CANVAS_COLS  64
#define CANVAS_ROWS  32
#define WINDOW_TITLE "Chip 8"

static u64 __time_ms__(void);

Renderer
Renderer_init(i32 scale)
{
    Renderer self = {};

    const i32 WINDIW_X_POS    = 0;
    const i32 WINDIW_Y_POS    = 0;
    const u32 WINDOW_FLAGS    = 0;
    const u32 SDL_FLAGS       = SDL_INIT_VIDEO;
    const u32 RENDERER_FLAGS  = 0;

    self.width  = CANVAS_COLS * scale;
    self.height = CANVAS_ROWS * scale;
    self.scale  = scale;

    i32 sdl_init_status = SDL_Init(SDL_FLAGS);

    if(sdl_init_status != 0)
    {
        fputs(SDL_GetError(), stderr);
        self.valid = false;
        return self;
    }

    self.window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        self.width - scale,     // for some unknown reason(s) we should remove that last col/row
        self.height - scale,
        WINDOW_FLAGS
    );

    if(!self.window)
    {
        fputs(SDL_GetError(), stderr);
        self.valid = false;
        return self;
    }

    self.sdl_renderer = SDL_CreateRenderer(self.window, -1, RENDERER_FLAGS);

    self.valid = true;
    self.display = calloc(CANVAS_COLS * CANVAS_ROWS, sizeof(u8));

    if(!self.display)
    {
        self.valid = false;
        return self;
    }

    self.valid = true;
    return self;
}

void
Renderer_render(Renderer* self)
{
    if(!self || !self->valid)
    {
        return;
    }

    // Set the draw color of renderer to green
    SDL_RenderClear(self->sdl_renderer);

    // rectangle color -> black
    SDL_SetRenderDrawColor(self->sdl_renderer, 0, 0, 0, 255);

    u32 x, y;
    for(int iii = 0; iii < CANVAS_COLS * CANVAS_ROWS; ++iii)
    {
        // If the value at this.display[i] == 1, then draw a pixel.
        if (self->display[iii] != 0)
        {
            // Grabs the x position of the pixel based off of `iii`
            x = (iii % CANVAS_COLS) * self->scale;

            // Grabs the y position of the pixel based off of `i`
            y = (iii / CANVAS_COLS) * self->scale;

            // Place a pixel at position (x, y) with a width and height of scale
            SDL_RenderFillRect(self->sdl_renderer, &(SDL_Rect){ x, y, self->scale, self->scale });
        }
    }

    // background color
    SDL_SetRenderDrawColor(self->sdl_renderer, 255, 255, 255, 255);

    // Update the renderer which will show the renderer cleared by the draw color which is green
    SDL_RenderPresent(self->sdl_renderer);
}

bool
Renderer_toggle_pixel(Renderer* self, u32 pos_x, u32 pos_y)
{
    if(!self->valid)
    {
        return false;
    }

    if (pos_x > CANVAS_COLS) {
        pos_x -= CANVAS_COLS;
    } else if (pos_x < 0) {
        pos_x += CANVAS_COLS;
    }

    if (pos_y > CANVAS_ROWS) {
        pos_y -= CANVAS_ROWS;
    } else if (pos_y < 0) {
        pos_y += CANVAS_ROWS;
    }

    u32 pixel_loc = pos_x + (pos_y * CANVAS_COLS);
    self->display[pixel_loc] ^= 1;

    return !self->display[pixel_loc];
}

void
Renderer_clear(Renderer* self)
{
    if(!self->valid)
    {
        return;
    }

    memset(self->display, 0, CANVAS_COLS * CANVAS_ROWS);
}

void
Renderer_deinit(Renderer* self)
{
    if(!self->valid)
    {
        fputs("Warning: deinitialize invalid Renderer", stderr);
    }

    if(self->window)
    {
        if(self->sdl_renderer)
        {
            //Destroy the renderer created above
            SDL_DestroyRenderer(self->sdl_renderer);
        }

        // Close and destroy the window
        SDL_DestroyWindow(self->window);
    }

    if(self->display)
    {
        free(self->display);
    }

    self->valid = false;
}


// Private functions
static u64 __time_ms__(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((u64)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}