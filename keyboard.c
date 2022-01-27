#include "keyboard.h"

#include <SDL2/SDL.h>

static bool
Keyboard__init_key__(MapI32* keys_map, u8 chip8_key, u8 key_code);

Keyboard
Keyboard_init()
{
    Keyboard keyboard = {};
    keyboard.keys_map = MapI32_construct(CHIP8_KEYS_COUNT);

    if(!keyboard.keys_map.valid)
    {
        keyboard.valid = false;
        return keyboard;
    }

    keyboard.chip8_keys_state = calloc(CHIP8_KEYS_COUNT, sizeof(u8));

    if(!keyboard.chip8_keys_state)
    {
        keyboard.valid = false;
        return keyboard;
    }

    if(!Keyboard__init_key__(&keyboard.keys_map, 0x1, SDLK_1)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0x2, SDLK_2)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0x3, SDLK_3)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0xc, SDLK_4)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0x4, SDLK_q)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0x5, SDLK_w)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0x6, SDLK_e)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0xD, SDLK_r)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0x7, SDLK_a)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0x8, SDLK_s)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0x9, SDLK_d)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0xE, SDLK_f)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0xA, SDLK_z)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0x0, SDLK_x)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0xB, SDLK_c)) return keyboard;
    if(!Keyboard__init_key__(&keyboard.keys_map, 0xF, SDLK_v)) return keyboard;

    keyboard.quit_pressed = false;
    keyboard.handler = NULL;
    keyboard.handler_arg = NULL;
    keyboard.valid = true;

    return keyboard;
}

// void
// Keyboard_register(Keyboard* self, SDL_KeyCode keycode, void(*handler)(void*))
// {
//     if(!self || !self->valid)
//     {
//         return;
//     }

//     if(!handler)
//     {
//         fputs("empty handler", stderr);
//     }

//     bool insertion_status = MapI32_insert(&self->keys_map, keycode, handler);

//     if(insertion_status)
//     {
//         fprintf(s.tderr, "Error %d: Couldn't register keycode %d", self->keys_map.error, keycode);
//         return;
//     }
// }

void
Keyboard_register(Keyboard* self, void(*handler)(void*, u8), void* handler_arg)
{
    self->handler = handler;
    self->handler_arg = handler_arg;
}

bool
keyboard_is_pressed(Keyboard* self, u8 chip8_key)
{
    return self->chip8_keys_state[chip8_key];
}

bool
Keyboard_is_quit_pressed(Keyboard* self)
{
    return self->quit_pressed;
}

void
Keyboard_run(Keyboard* self)
{
    if(!self || !self->valid)
    {
        return;
    }

    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN)
        {
            const u8* key = MapI32_search(&self->keys_map, event.key.keysym.sym);
            if(key)
            {
                self->chip8_keys_state[*key] = true;

                if(self->handler)
                {
                    self->handler(self->handler_arg, *key);
                    self->handler = NULL;
                    self->handler_arg = NULL;
                }
            }
        }
        else if (event.type == SDL_KEYUP)
        {
            const u8* key = MapI32_search(&self->keys_map, event.key.keysym.sym);
            if(key)
            {
                self->chip8_keys_state[*key] = false;
            }
        }
        else if (event.type == SDL_QUIT)
        {
            self->quit_pressed = true;
        }
    }
}

void
Keyboard_deinit(Keyboard* self)
{
    if(!self)
    {
        return;
    }

    free(self->chip8_keys_state);
    MapI32_deconstruct(&self->keys_map);

    self->valid = false;
}

// private functions
bool
Keyboard__init_key__(MapI32* keys_map, u8 chip8_key, u8 key_code)
{
    if(!MapI32_insert(keys_map, key_code, chip8_key))
    {
        fprintf(stderr, "Error %d: Couldn't register keycode %d", 
            keys_map->error,
            chip8_key
        );
        return false;
    }

    return true;
}