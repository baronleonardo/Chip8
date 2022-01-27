#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "utils/type_alias.h"
#include "utils/map.h"

#include <stdbool.h>

#include <SDL2/SDL_keycode.h>

#define CHIP8_KEYS_COUNT    16

// typedef struct {
//     u8 chip8_key;
//     // bool is_pressed;
//     // void(*handler)(void*);
// } Keyboard_Key ;

typedef struct {
    MapI32 keys_map;
    // u8* chip8_keys;
    u8* chip8_keys_state;
    bool quit_pressed;
    bool valid;
    void(*handler)(void*, u8);
    void* handler_arg;
} Keyboard;

Keyboard
Keyboard_init();

/// @NOTE: this is a one shot handler, it means it will be called one
///        if you want to be fired again you need to register again
/// @ARG: handler:
///             void*: handler_arg
///             u8   : key_code
void
Keyboard_register(Keyboard* self, void(*handler)(void*, u8), void* handler_arg);

bool
keyboard_is_pressed(Keyboard* self, u8 chip8_key);

bool
Keyboard_is_quit_pressed(Keyboard* self);

void
Keyboard_run(Keyboard* self);

void
Keyboard_deinit(Keyboard* self);

#endif // KEYBOARD_H