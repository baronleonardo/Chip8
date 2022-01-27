#include "chip8.h"
#include "string.h"

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "%s: you should pass the rom file\n", argv[0]);
        exit(0);
    }

    String rom_path = String_from_char_ptr(argv[1]);

    u8 chip8_speed = 15;
    u8 chip8_scale = 10;

    Chip8 chip8 = Chip8_init(rom_path, chip8_scale, chip8_speed);

    Chip8_mainloop(&chip8);

    Chip8_deinit(&chip8);
}