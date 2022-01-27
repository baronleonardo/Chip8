#ifndef CPU_H
#define CPU_H

#include "utils/type_alias.h"
#include "utils/string.h"
#include "utils/stack.h"
#include "renderer.h"
#include "keyboard.h"
#include "speaker.h"

#include <stdbool.h>

typedef enum {
    CPU_NO_ERROR,
    CPU_ERROR_INVALID_SELF,
    CPU_ERROR_INVALID_INSTRUCTION,
    CPU_ERROR_INVALID_PROGRAM,
} Cpu_Error;

typedef struct Cpu Cpu;

typedef struct {
    bool (*run)(Cpu* cpu, u16 opcode);
} Cpu_Instruction;

typedef struct Cpu {
    struct {
        u8* memory;
        u8* registers;
        u16 i;
        u16 delay_timer;
        u16 sound_timer;
        u16 pc; // program counter
        Stack stack;
        bool paused;
        u8 speed;
    };

    bool valid;
    bool has_valid_rom;
    i32 error;
    Cpu_Instruction* instructions;
    u16 current_instruction;
    Renderer* renderer;
    Keyboard* keyboard;
    Speaker* speaker;
} Cpu;

Cpu
Cpu_init(Renderer* renderer, Keyboard* keyboard, Speaker* speaker, u8 speed);

void
Cpu_load_program(Cpu* self, u8* program, size_t program_size);

// it will abort if invalid instruction found
void
Cpu_cycle(Cpu* self);

void
Cpu_deinit(Cpu* self);

#endif // CPU_H