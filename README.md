# YACHE - Yet Another CHIP-8 Emulator

Here is a CHIP-8 emulator.

This project was made for fun. There is plenty of better CHIP-8 implementations out there ;-).

For more information about CHIP-8, visit [Matthew Mikolay CHIP-8 Wiki](https://github.com/mattmikolay/chip-8/wiki).

Running Brix by Andreas Gustafsson:
![Screenshot](https://user-images.githubusercontent.com/7930348/199541437-028be0c3-443e-4442-9f3e-ba56064d89d9.png)

## Compilation

The only dependency you need is SDL2.
You also need CMake for building the sources:

```bash
mkdir build
cd build
cmake ..
make
```

## Execution

You need to pass a ROM path as argument of the program:

```bash
./yache ./roms/test.ch8
```

You can find a good collection of ROMs here:

- https://github.com/kripod/chip8-roms
- https://johnearnest.github.io/chip8Archive/


The controls are mapped as follows on an US QWERTY keyboard:

```
 ╔═══╦═══╦═══╦═══╗
 ║ 1 ║ 2 ║ 3 ║ 4 ║
 ╠═══╬═══╬═══╬═══╣
 ║ Q ║ W ║ E ║ R ║
 ╠═══╬═══╬═══╬═══╣
 ║ A ║ S ║ D ║ F ║
 ╠═══╬═══╬═══╬═══╣
 ║ Z ║ X ║ C ║ V ║
 ╚═══╩═══╩═══╩═══╝
```

## Implementation variations

Some instructions vary depending on the CHIP-8 implementation. You can change some variation by commending or uncommenting the following defines in `src/computer.h`:

```C
#define ALT_SHIFT
#define ALT_STR_LD
```

`ALT_SHIFT` changes the behaviour of instructions `8XY6` and `8XYE`.

- When enabled, the shift is applied to `vx` register and saved in `vx`. `vy` is not used.
- When disabled, the shift is applied to `vy` register and saved in `vx`.

`ALT_STR_LD` changes the behaviour of instructions `FX55` and `FX65`.

- When enabled, `I` register is incremented by `X + 1` after the execution.
- When disabled, `I` register remains unchanged after the execution.

You may want to change the state of the two defines depending on the ROM you want to run. 
