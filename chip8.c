#include "chip8.h"

#include <sys/time.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    u8* data;
    size_t size;
} Chip8__Rom__;

static Chip8__Rom__
Chip8__load_rom__(Chip8* self, String rom_path);

static u64
__time_ms__(void);

static void
Chip8__on_quit__(void* arg);

Chip8
Chip8_init(String rom_path, u8 screen_scale, u8 speed)
{
    Chip8 chip8 = {};

    chip8.fps = 60;
    chip8.fps_interval = 1000 / chip8.fps;
    chip8.is_running = true;

    u64 start_time = __time_ms__();
    chip8.past_time = chip8.start_time;
    chip8.now = chip8.start_time;

    chip8.keyboard = malloc(sizeof(Keyboard));
    chip8.renderer = malloc(sizeof(Renderer));
    chip8.speaker = malloc(sizeof(Speaker));

    if(!chip8.keyboard || !chip8.renderer || !chip8.speaker)
    {
        chip8.valid = false;
        return chip8;
    }

    *chip8.keyboard = Keyboard_init();
    *chip8.renderer = Renderer_init(screen_scale);
    *chip8.speaker = Speaker_init();
    chip8.cpu = Cpu_init(chip8.renderer, chip8.keyboard, chip8.speaker, speed);

    Chip8__Rom__ rom = Chip8__load_rom__(&chip8, rom_path);

    Cpu_load_program(&chip8.cpu, rom.data, rom.size);
    free(rom.data);

    chip8.valid = true;
    return chip8;
}

void
Chip8_mainloop(Chip8* self)
{
    while(!self->keyboard->quit_pressed)
    {
        Cpu_cycle(&self->cpu);
        SDL_Delay(self->fps_interval);
    }
}

void
Chip8_deinit(Chip8* self)
{
    if(!self)
    {
        return;
    }

    Keyboard_deinit(self->keyboard);
    Renderer_deinit(self->renderer);
    Speaker_deinit(self->speaker);

    free(self->keyboard);
    free(self->renderer);
    free(self->speaker);

    // Clean up SDL2 and exit the program
    SDL_Quit();
}


// Private functions
static u64
__time_ms__(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((u64)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

void
Chip8__on_quit__(void* arg)
{
    if(arg == NULL)
    {
        puts("ylahwaaaaaaaay");
    }
    ((Chip8*)arg)->is_running = false;
}

Chip8__Rom__
Chip8__load_rom__(Chip8* self, String rom_path)
{
    Chip8__Rom__ rom = {};

    if(rom_path.len == 0)
    {
        fputs("Error: CPU: error loading rom_file", stderr);
        abort();
    }

    FILE* rom_file = fopen(rom_path.data, "rb");
    if(!rom_file)
    {
        fprintf(stderr, "Couldn't open %s\n", rom_path.data);
        abort();
    }

    // obtain file size:
    fseek (rom_file , 0 , SEEK_END);
    rom.size = ftell (rom_file);
    rewind (rom_file);

    // allocate memory to contain the whole file:
    rom.data = (u8*) malloc (sizeof(char) * rom.size);
    if (rom.data == NULL)
    {
        fputs("Couldn't allocate memory for the program", stderr);
        abort();
    }

    rom.size = fread(
        rom.data,
        sizeof(u8),
        rom.size,
        rom_file
    );

    fclose(rom_file);

    return rom;
}