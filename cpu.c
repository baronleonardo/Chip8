#include "cpu.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define CHIP8_MEM           4096
#define CHIP8_REGS          16
#define CHIP8_INIT_PC_ADDR  0x200
#define CHIP8_MAX_ROM_SIZE  0xDFF
#define CHIP8_INSTERUCTIONS 16
#define CHIP8_SPRITES_SIZE  80
#define CHIP8_STACK_SIZE    16

#define BITS_PER_BYTE       8

static bool
Cpu__execute__(Cpu* self, u16 opcode);

static void
Cpu__update_timers__(Cpu* self);

static void
Cpu__play_sound__(Cpu* self);

static bool
Cpu__on_0x0(Cpu* self, u16 opcode);

static bool
Cpu__on_0x1(Cpu* self, u16 opcode);

static bool
Cpu__on_0x2(Cpu* self, u16 opcode);

static bool
Cpu__on_0x3(Cpu* self, u16 opcode);

static bool
Cpu__on_0x4(Cpu* self, u16 opcode);

static bool
Cpu__on_0x5(Cpu* self, u16 opcode);

static bool
Cpu__on_0x6(Cpu* self, u16 opcode);

static bool
Cpu__on_0x7(Cpu* self, u16 opcode);

static bool
Cpu__on_0x8(Cpu* self, u16 opcode);

static bool
Cpu__on_0x9(Cpu* self, u16 opcode);

static bool
Cpu__on_0xA(Cpu* self, u16 opcode);

static bool
Cpu__on_0xB(Cpu* self, u16 opcode);

static bool
Cpu__on_0xC(Cpu* self, u16 opcode);

static bool
Cpu__on_0xD(Cpu* self, u16 opcode);

static bool
Cpu__on_0xE(Cpu* self, u16 opcode);

static bool
Cpu__on_0xF(Cpu* self, u16 opcode);

static void
Cpu__on_pause(Cpu* self, u8 key);

Cpu
Cpu_init(Renderer* renderer, Keyboard* keyboard, Speaker* speaker, u8 speed)
{
    Cpu cpu = {};

    if(!renderer || !renderer->valid ||
       !keyboard || !keyboard->valid ||
       !speaker  || !speaker->valid)
    {
        fputs("Error: CPU: renderer or keyboard or speaker are invalid", stderr);
        cpu.valid = false;
        return cpu;
    }

    cpu.memory = malloc(CHIP8_MEM);
    cpu.registers = calloc(CHIP8_REGS, sizeof(u8));
    cpu.instructions = calloc(CHIP8_INSTERUCTIONS, sizeof(Cpu_Instruction));
    cpu.stack = Stack_construct(CHIP8_STACK_SIZE, true);
    if(!cpu.memory || !cpu.registers || !cpu.instructions || !cpu.stack.valid)
    {
        cpu.valid = false;
        return cpu;
    }

    cpu.i = 0;
    cpu.delay_timer = 0;
    cpu.sound_timer = 0;
    cpu.pc = CHIP8_INIT_PC_ADDR; // program counter
    cpu.paused = false;
    cpu.speed = speed;

    // set sprites (screen) in memory starting from address 0x0
    memcpy(
        cpu.memory, 
        (u8[]) {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        },
        CHIP8_SPRITES_SIZE
    );

    // Cpu instructions handlers
    memcpy(
        cpu.instructions,
        (bool (*[])(Cpu*, u16)) {
            Cpu__on_0x0,
            Cpu__on_0x1,
            Cpu__on_0x2,
            Cpu__on_0x3,
            Cpu__on_0x4,
            Cpu__on_0x5,
            Cpu__on_0x6,
            Cpu__on_0x7,
            Cpu__on_0x8,
            Cpu__on_0x9,
            Cpu__on_0xA,
            Cpu__on_0xB,
            Cpu__on_0xC,
            Cpu__on_0xD,
            Cpu__on_0xE,
            Cpu__on_0xF
        },
        CHIP8_INSTERUCTIONS * sizeof(Cpu_Instruction)
    );

    cpu.renderer = renderer;
    cpu.keyboard = keyboard;
    cpu.speaker = speaker;
    cpu.current_instruction = 0;
    cpu.error = CPU_NO_ERROR;
    cpu.has_valid_rom = false;
    cpu.valid = true;
    return cpu;
}

void
Cpu_load_program(Cpu* self, u8* program, size_t program_size)
{
    if(!self || !self->valid)
    {
        self->error = CPU_ERROR_INVALID_SELF;
        return;
    }

    if(!program || program_size == 0 || program_size > CHIP8_MAX_ROM_SIZE)
    {
        fputs("Error: CPU: program is NULL or has bad size", stderr);
        self->error = CPU_ERROR_INVALID_PROGRAM;
        return;
    }

    memcpy(&self->memory[CHIP8_INIT_PC_ADDR], program, program_size);

    self->has_valid_rom = true;
    self->error = CPU_NO_ERROR;
}

void
Cpu_cycle(Cpu* self)
{
    if(!self || !self->valid)
    {
        self->error = CPU_ERROR_INVALID_SELF;
        return;
    }

    for(int iii = 0; iii < self->speed; iii++)
    {
        if(!self->paused)
        {
            u16 opcode = ((self->memory[self->pc] << BITS_PER_BYTE) | self->memory[self->pc + 1]);
            if(!Cpu__execute__(self, opcode))
            {
                fprintf(stderr, "Error: Cpu: wrong opcode %x\n", opcode);
                abort();
            }
        }
    }

    if(!self->paused)
    {
        Cpu__update_timers__(self);
    }

    
    Keyboard_run(self->keyboard);
    Cpu__play_sound__(self);
    Renderer_render(self->renderer);

    self->error = CPU_NO_ERROR;
}

void
Cpu_deinit(Cpu* self)
{
    if(!self)
    {
        return;
    }

    free(self->memory);
    free(self->registers);
    free(self->instructions);
}

// private functions
bool
Cpu__execute__(Cpu* self, u16 opcode)
{
    // Increment the program counter to prepare it for the next instruction.
    // Each instruction is 2 bytes long, so increment it by 2.
    self->pc += 2;

    u8 instruction = ((opcode & 0xF000) >> 12);

    if(self->instructions[instruction].run(self, opcode))
    {
        self->error = CPU_NO_ERROR;
        return true;
    }
    else
    {
        self->error = CPU_ERROR_INVALID_INSTRUCTION;
        return false;
    }
}

void
Cpu__update_timers__(Cpu* self)
{
    if (self->delay_timer > 0)
    {
        self->delay_timer--;
    }

    if (self->sound_timer > 0)
    {
        self->sound_timer--;
    }
}

void
Cpu__play_sound__(Cpu* self)
{
    if (self->sound_timer > 0)
    {
        Speaker_play(self->speaker, 440, -1);
    }
    else
    {
        Speaker_stop(self->speaker);
    }
}

bool
Cpu__on_0x0(Cpu* self, u16 opcode)
{

    switch (opcode)
    {
        case 0x00E0:
            Renderer_clear(self->renderer);
            break;
        case 0x00EE:
        {
            u16 pop = Stack_pop(&self->stack);
            if(self->error != CPU_NO_ERROR) return false;
            self->pc = pop; 
            break;
        }
        default:
            return false;
    }

    return true;
}

bool
Cpu__on_0x1(Cpu* self, u16 opcode)
{

    self->pc = (opcode & 0xFFF);
    return true;
}

bool
Cpu__on_0x2(Cpu* self, u16 opcode)
{

    Stack_push(&self->stack, self->pc);
    if(self->error != STACK_NO_ERROR) return false;

    self->pc = (opcode & 0xFFF);
    return true;
}

bool
Cpu__on_0x3(Cpu* self, u16 opcode)
{
    u8 x = (opcode & 0x0F00) >> 8;

    if (self->registers[x] == (opcode & 0xFF))
    {
        self->pc += 2;
    }

    return true;
}

bool
Cpu__on_0x4(Cpu* self, u16 opcode)
{
    u8 x = (opcode & 0x0F00) >> 8;

    if (self->registers[x] != (opcode & 0xFF))
    {
        self->pc += 2;
    }

    return true;
}

bool
Cpu__on_0x5(Cpu* self, u16 opcode)
{
    u8 x = (opcode & 0x0F00) >> 8;
    u8 y = (opcode & 0x00F0) >> 4;

    if (self->registers[x] == self->registers[y])
    {
        self->pc += 2;
    }
    return true;
}

bool
Cpu__on_0x6(Cpu* self, u16 opcode)
{
    u8 x = (opcode & 0x0F00) >> 8;

    self->registers[x] = (opcode & 0xFF);
    return true;
}

bool
Cpu__on_0x7(Cpu* self, u16 opcode)
{
    u8 x = (opcode & 0x0F00) >> 8;

    self->registers[x] += (opcode & 0xFF);
    return true;
}

bool
Cpu__on_0x8(Cpu* self, u16 opcode)
{
    u8 x = (opcode & 0x0F00) >> 8;
    u8 y = (opcode & 0x00F0) >> 4;

    switch(opcode & 0xF)
    {
        case 0x0:
            self->registers[x] = self->registers[y];
            break;
        case 0x1:
            self->registers[x] |= self->registers[y];
            break;
        case 0x2:
            self->registers[x] &= self->registers[y];
            break;
        case 0x3:
            self->registers[x] ^= self->registers[y];
            break;
        case 0x4:
            {
                u16 sum = (self->registers[x] += self->registers[y]);

                self->registers[0xF] = 0;

                if (sum > 0xFF) {
                    self->registers[0xF] = 1;
                }

                self->registers[x] = (u8)sum;
                break;
            }
        case 0x5:
            {
                self->registers[0xF] = 0;

                if (self->registers[x] > self->registers[y]) 
                {
                    self->registers[0xF] = 1;
                }

                self->registers[x] -= self->registers[y];
                break;
            }
        case 0x6:
            self->registers[0xF] = (self->registers[x] & 0x1);
            self->registers[x] >>= 1;
            break;
        case 0x7:
            {
                self->registers[0xF] = 0;

                if (self->registers[y] > self->registers[x]) {
                    self->registers[0xF] = 1;
                }

                self->registers[x] = self->registers[y] - self->registers[x];
                break;
            }
        case 0xE:
            self->registers[0xF] = (self->registers[x] & 0x80);
            self->registers[x] <<= 1;
            break;
    }

    return true;
}

bool
Cpu__on_0x9(Cpu* self, u16 opcode)
{
    u8 x = (opcode & 0x0F00) >> 8;
    u8 y = (opcode & 0x00F0) >> 4;

    if (self->registers[x] != self->registers[y])
    {
        self->pc += 2;
    }

    return true;
}

bool
Cpu__on_0xA(Cpu* self, u16 opcode)
{

    self->i = (opcode & 0xFFF);
    return true;
}

bool
Cpu__on_0xB(Cpu* self, u16 opcode)
{

    self->pc = (opcode & 0xFFF) + self->registers[0];
    return true;
}

bool
Cpu__on_0xC(Cpu* self, u16 opcode)
{

    time_t t1;
    srand((u32)time(&t1));
    u32 rand_num = rand() % 0xFF;

    u8 x = (opcode & 0x0F00) >> 8;
    self->registers[x] = rand_num & (opcode & 0xFF);

    return true;
}

bool
Cpu__on_0xD(Cpu* self, u16 opcode)
{
    u8 width = 8;
    u8 height = (opcode & 0xF);
    u8 x = (opcode & 0x0F00) >> 8;
    u8 y = (opcode & 0x00F0) >> 4;

    self->registers[0xF] = 0;

    for (u8 row = 0; row < height; row++)
    {
        u8 sprite = self->memory[self->i + row];

        for (u8 col = 0; col < width; col++)
        {
            // If the bit (sprite) is not 0, render/erase the pixel
            if ((sprite & 0x80) > 0)
            {
                // If setPixel returns 1, which means a pixel was erased, set VF to 1
                if (Renderer_toggle_pixel(self->renderer, self->registers[x] + col, self->registers[y] + row))
                {
                    self->registers[0xF] = 1;
                }
            }

            // Shift the sprite left 1. This will move the next next col/bit of the sprite into the first position.
            // Ex. 10010000 << 1 will become 0010000
            sprite <<= 1;
        }
    }

    return true;
}

bool
Cpu__on_0xE(Cpu* self, u16 opcode)
{
    u8 x = (opcode & 0x0F00) >> 8;

    switch (opcode & 0xFF)
    {
        case 0x9E:
            if (keyboard_is_pressed(self->keyboard, self->registers[x]))
            {
                self->pc += 2;
            }
        case 0xA1:
            if (!keyboard_is_pressed(self->keyboard, self->registers[x]))
            {
                self->pc += 2;
            }
    }

    return true;
}

bool
Cpu__on_0xF(Cpu* self, u16 opcode)
{
    u8 x = (opcode & 0x0F00) >> 8;
    u8 y = (opcode & 0x00F0) >> 4;
    self->current_instruction = opcode;

    switch (opcode & 0xFF)
    {
        case 0x07:
            self->registers[x] = self->delay_timer;
            break;
        case 0x0A:
            self->paused = true;
            Keyboard_register(self->keyboard, (void (*)(void *, u8))Cpu__on_pause, self);
            break;
        case 0x15:
            self->delay_timer = self->registers[x];
            break;
        case 0x18:
            self->sound_timer = self->registers[x];
            break;
        case 0x1E:
            self->i += self->registers[x];
            break;
        case 0x29:
            // location of sprite
            self->i = self->registers[x] * 5;
            break;
        case 0x33:
            // Get the hundreds digit and place it in I.
            self->memory[self->i] = self->registers[x] / 100;

            // Get tens digit and place it in I+1. Gets a value between 0 and 99,
            // then divides by 10 to give us a value between 0 and 9.
            self->memory[self->i + 1] = (self->registers[x] % 100) / 10;

            // Get the value of the ones (last) digit and place it in I+2.
            self->memory[self->i + 2] = self->registers[x] % 10;
            break;
        case 0x55:
            for (u8 registerIndex = 0; registerIndex <= x; registerIndex++)
            {
                self->memory[self->i + registerIndex] = self->registers[registerIndex];
            }
            break;
        case 0x65:
            for (u8 registerIndex = 0; registerIndex <= x; registerIndex++)
            {
                self->registers[registerIndex] = self->memory[self->i + registerIndex];
            }
            break;
    }

    return true;
}

void
Cpu__on_pause(Cpu* self, u8 key)
{
    u8 x = (self->current_instruction & 0x0F00) >> 8;
    self->registers[x] = key;
    self->paused = false;
}