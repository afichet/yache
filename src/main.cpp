#include <iostream>
#include <cstdio>
#include <cstdint>
#include <vector>

#include <computer.h>
#include <beeper.h>

#include <SDL.h>
#include <cassert>
#include <cstring>

// Tests
// - https://github.com/Skosulor/c8int/tree/master/test
// - https://github.com/corax89/chip8-test-rom

// Converts 1 bit per pixel screen data to RGBA
void screen_to_sdl(
    const std::vector<uint8_t>& screen,
    int width, int height,
    std::vector<uint8_t>& sdl_screen,
    int width_sdl, int height_sdl)
{

    assert(screen.size() == (width / 8) * height);
    assert(sdl_screen.size() == 4 * width_sdl * height_sdl);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width / 8; x++) {
            const int x_sdl = 8 * x;

            // Get 8 px at a time
            uint8_t c = screen[y * width / 8 + x];

            uint8_t offset = 0b10000000;

            for (int i = 0; i < 8; i++) {
                sdl_screen[4 * (y * width_sdl + (8 * x + i)) + 0] = (c & offset) ? 0xFF: 0x00;
                sdl_screen[4 * (y * width_sdl + (8 * x + i)) + 1] = (c & offset) ? 0xFF: 0x00;
                sdl_screen[4 * (y * width_sdl + (8 * x + i)) + 2] = (c & offset) ? 0xFF: 0x00;
                sdl_screen[4 * (y * width_sdl + (8 * x + i)) + 3] = 0xFF;

                offset = offset >> 1;
            }
        }
    }
}


// Create key bindings
// ╔═══╦═══╦═══╦═══╗
// ║ 1 ║ 2 ║ 3 ║ C ║
// ╠═══╬═══╬═══╬═══╣
// ║ 4 ║ 5 ║ 6 ║ D ║
// ╠═══╬═══╬═══╬═══╣
// ║ 7 ║ 8 ║ 9 ║ E ║
// ╠═══╬═══╬═══╬═══╣
// ║ A ║ 0 ║ B ║ F ║
// ╚═══╩═══╩═══╩═══╝
uint8_t keyBinding(SDL_Scancode code)
{
    // !(1) @(2) #(3) $(4)  for US QWERTY
    if (code == 30) { return 0x1; }
    if (code == 31) { return 0x2; }
    if (code == 32) { return 0x3; }
    if (code == 33) { return 0xC; }

    // QWER                 for US QWERTY
    if (code == 20) { return 0x4; }
    if (code == 26) { return 0x5; }
    if (code == 8)  { return 0x6; }
    if (code == 21) { return 0xD; }

    // ASDF                 for US QWERTY
    if (code == 4)  { return 0x7; }
    if (code == 22) { return 0x8; }
    if (code == 7)  { return 0x9; }
    if (code == 9)  { return 0xE; }

    // ZXCV                 for US QWERTY
    if (code == 29) { return 0xA; }
    if (code == 27) { return 0x0; }
    if (code == 6)  { return 0xB; }
    if (code == 25) { return 0xF; }

    // Not valid keycode
    return 255;
}


int main(int argc, char* argv[])
{
    const uint32_t proc_speed_Hz = 300;
    const uint32_t tick_length_ms = (uint32_t)(1.f/(float)proc_speed_Hz * 1000.f);

    if (argc < 2) {
        std::cout << "Usage:" << std::endl
                  << "------" << std::endl
                  << argv[0] << " <chip8_rom>" << std::endl;
        return 0;
    }

    const char* filename = argv[1];

    std::FILE *f_rom = std::fopen(filename, "rb");

    if (!f_rom) {
        std::cerr << "Could not open ROM file" << std::endl;
        return -1;
    }

    // Retrieve file size
    std::fseek(f_rom, 0L, SEEK_END);
    long int rom_size = std::ftell(f_rom);
    std::rewind(f_rom);

    // Allocate enough memory to store the rom
    std::vector<uint8_t> rom(rom_size);

    // Read rom content
    std::fread(rom.data(), rom_size, 1, f_rom);
    std::fclose(f_rom);

    // Initialize the CHIP-8 computer
    Computer computer(rom);

    // Start SDL
    SDL_Renderer* renderer;
    SDL_Window*   window;
    SDL_Texture*  texture;

    SDL_Event event;
    bool quit = false;

    int screen_w = computer.width();
    int screen_h = computer.height();
    int scale = 12;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL: "
                  << SDL_GetError() << std::endl;
        return -1;
    }

    window = SDL_CreateWindow("CHIP8",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        scale * screen_w,
        scale * screen_h,
        SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::cerr << "Failed to create a window: "
                  << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!renderer) {
        std::cerr << "Failed to create a renderer: "
                  << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_TARGET,
        screen_w, screen_h
    );

    if (!texture) {
        std::cerr << "Failed to create a texture: "
                  << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    std::vector<uint8_t> screen_texture(screen_w * screen_h * 4);

    uint64_t ticks_prev;
    uint8_t key_down, key_up;

    Beeper beeper(550., 0.);

    while (!quit) {
        // We keep track of the host computation loop duration
        ticks_prev = SDL_GetTicks64();

        while (SDL_PollEvent(&event)) {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    key_down = keyBinding(event.key.keysym.scancode);
                    if (key_down != 255) {
                        computer.keyPress(key_down);
                    }
                    break;
                case SDL_KEYUP:
                    key_up = keyBinding(event.key.keysym.scancode);
                    if (key_down != 255) {
                        computer.keyRelease(key_up);
                    }
                    break;
                default:
                    break;
            }
        }

        // CPU cycle
        computer.tick();

        // Convert the 1bit screen to RGBA
        screen_to_sdl(
            computer.screen(),
            screen_w, screen_h,
            screen_texture,
            screen_w, screen_h
        );

        SDL_UpdateTexture(
            texture, NULL,
            screen_texture.data(),
            screen_w * sizeof(uint32_t)
        );

        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // Sound management
        int beep_cycles_length = computer.soundTimer();
        float beep_duration_sec = (beep_cycles_length * tick_length_ms) / 1000.;

        // As noted in the COSMAC VIP manual, the minimum value that the timer
        // will respond to is 0x02. [4] Thus, setting the timer to a value of
        // 0x01 will have no audible effect.
        if (beep_cycles_length >= 0x02) {
            beep_duration_sec = std::min(beep_duration_sec, beeper.minDuration());
            beeper.setDurationLeft(beep_duration_sec);
            beeper.setPaused(0);
        }

        const uint64_t ticks_now = SDL_GetTicks64();
        SDL_Delay(tick_length_ms - (ticks_prev - ticks_now));
    }

    SDL_DestroyTexture(texture);
    SDL_Quit();

    return 0;
}