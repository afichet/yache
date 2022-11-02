#include "computer.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <stdexcept>

Computer::Computer(
    const std::vector<uint8_t>& program)
    : m_wait_for_key_press(false)
    , m_delay_timer(0)
    , m_sound_timer(0)
    , m_I_register(0)
    , m_program_counter(0x200)
    , m_screen(screen_width / 8 * screen_height)
{
    // Fist ensure the program can fit in ram
    const uint16_t ram_pgm_available = 0xE8F - 0x200 + 1;

    // Copy the program to ram
    if (program.size() > ram_pgm_available) {
        throw std::runtime_error("Progam is too big to fit in RAM");
    } else {
        std::memcpy(&m_memory[0x200], program.data(), program.size());
    }

    // Set the basic characters
    std::memcpy(&m_memory[0x0 * 5], char_0, 5);
    std::memcpy(&m_memory[0x1 * 5], char_1, 5);
    std::memcpy(&m_memory[0x2 * 5], char_2, 5);
    std::memcpy(&m_memory[0x3 * 5], char_3, 5);
    std::memcpy(&m_memory[0x4 * 5], char_4, 5);
    std::memcpy(&m_memory[0x5 * 5], char_5, 5);
    std::memcpy(&m_memory[0x6 * 5], char_6, 5);
    std::memcpy(&m_memory[0x7 * 5], char_7, 5);
    std::memcpy(&m_memory[0x8 * 5], char_8, 5);
    std::memcpy(&m_memory[0x9 * 5], char_9, 5);
    std::memcpy(&m_memory[0xA * 5], char_A, 5);
    std::memcpy(&m_memory[0xB * 5], char_B, 5);
    std::memcpy(&m_memory[0xC * 5], char_C, 5);
    std::memcpy(&m_memory[0xD * 5], char_D, 5);
    std::memcpy(&m_memory[0xE * 5], char_E, 5);
    std::memcpy(&m_memory[0xF * 5], char_F, 5);

    for (int i = 0; i < m_keypad.size(); i++) {
        m_keypad[i] = false;
    }
}


void Computer::keyPress(uint8_t key)
{
    m_last_key_pressed = key;
    m_keypad[key] = true;
}


void Computer::keyRelease(uint8_t key) {
    m_keypad[key] = false;
}


void Computer::tick()
{
    // Retrieve the instruction from memory
    uint16_t instruction = m_memory[m_program_counter] << 8 | m_memory[m_program_counter + 1];

    // Sound timer
    if (m_sound_timer > 0) {
        m_sound_timer -= 1;
    }

    if (m_delay_timer == 0) {
        exec(instruction);
    } else {
        m_delay_timer -= 1;
    }
}


// Execute machine language subroutine at address NNN
void Computer::inst_0NNN(uint16_t addr)
{
    #ifdef PRINT_OPCODE
    std::cout << "Exec_natives";
    #endif

    // FIXME: not implemented
    // https://github.com/mattmikolay/chip-8/wiki/Mastering-CHIP%E2%80%908#subroutines
    std::cout << "??" << std::endl;
    m_program_counter += 2;
}


// Clear the screen
void Computer::inst_00E0()
{
    #ifdef PRINT_OPCODE
    std::cout << "CLR_SCR";
    #endif

    std::memset(&m_screen[0], 0, screen_width / 8 * screen_height);

    m_program_counter += 2;
}


// Return from a subroutine
void Computer::inst_00EE()
{
    #ifdef PRINT_OPCODE
    std::cout << "RET";
    #endif

    m_program_counter = m_stack.back();
    m_stack.pop_back();

    m_program_counter += 2;
}


// Jump to address NNN
void Computer::inst_1NNN(uint16_t addr)
{
    #ifdef PRINT_OPCODE
    std::cout << "JMP " << std::hex << addr;
    #endif

    m_program_counter = addr;
}


// Execute subroutine starting at address NNN
void Computer::inst_2NNN(uint16_t addr)
{
    #ifdef PRINT_OPCODE
    std::cout << "CALL 0x" << std::hex << addr;
    #endif

    m_stack.push_back(m_program_counter);
    m_program_counter = addr;
}


// Skip the following instruction if the value of register VX equals NN
void Computer::inst_3XNN(uint8_t reg_x, uint8_t value)
{
    #ifdef PRINT_OPCODE
    std::cout << "SKIP_EQ v" << std::hex << (int)(reg_x) << " " << int(value);
    #endif

    if (m_registers[reg_x] == value) {
        m_program_counter += 4;
    } else {
        m_program_counter += 2;
    }
}


// Skip the following instruction if the value of register VX is not equal to NN
void Computer::inst_4XNN(uint8_t reg_x, uint8_t value)
{
    #ifdef PRINT_OPCODE
    std::cout << "SKIP_NEQ v" << std::hex << (int)(reg_x) << " " << int(value);
    #endif

    if (m_registers[reg_x] != value) {
        m_program_counter += 4;
    } else {
        m_program_counter += 2;
    }
}


// Skip the following instruction if the value of register VX is equal to the
// value of register VY
void Computer::inst_5XY0(uint8_t reg_x, uint8_t reg_y)
{
    #ifdef PRINT_OPCODE
    std::cout << "SKIP_EQ v" << std::hex << (int)(reg_x) << " v" << std::hex << (int)(reg_y);
    #endif

    if (m_registers[reg_x] == m_registers[reg_y]) {
        m_program_counter += 4;
    } else {
        m_program_counter += 2;
    }
}


// Store number NN in register VX
void Computer::inst_6XNN(uint8_t reg_x, uint8_t value)
{
    #ifdef PRINT_OPCODE
    std::cout << "STR v" << std::hex << (int)(reg_x) << " " << (int)(value);
    #endif

    m_registers[reg_x] = value;

    m_program_counter += 2;
}


// Add the value NN to register VX
void Computer::inst_7XNN(uint8_t reg_x, uint8_t value)
{
    #ifdef PRINT_OPCODE
    std::cout << "ADD v" << std::hex << (int)(reg_x) << " " << (int)(value);
    #endif

    m_registers[reg_x] += value;

    m_program_counter += 2;
}


// Store the value of register VY in register VX
void Computer::inst_8XY0(uint8_t reg_x, uint8_t reg_y)
{
    #ifdef PRINT_OPCODE
    std::cout << "STR v" << std::hex << (int)(reg_x) << " v" << std::hex << (int)(reg_y);
    #endif

    m_registers[reg_x] = m_registers[reg_y];

    m_program_counter += 2;
}

// Set VX to VX OR VY
void Computer::inst_8XY1(uint8_t reg_x, uint8_t reg_y)
{
    #ifdef PRINT_OPCODE
    std::cout << "OR v" << std::hex << (int)(reg_x) << " v" << std::hex << (int)(reg_y);
    #endif

    m_registers[reg_x] |= m_registers[reg_y];

    m_program_counter += 2;
}


// Set VX to VX AND VY
void Computer::inst_8XY2(uint8_t reg_x, uint8_t reg_y)
{
    #ifdef PRINT_OPCODE
    std::cout << "AND v" << std::hex << (int)(reg_x) << " v" << std::hex << (int)(reg_y);
    #endif

    m_registers[reg_x] &= m_registers[reg_y];

    m_program_counter += 2;
}


// Set VX to VX XOR VY
void Computer::inst_8XY3(uint8_t reg_x, uint8_t reg_y)
{
    #ifdef PRINT_OPCODE
    std::cout << "XOR v" << std::hex << (int)(reg_x) << " v" << std::hex << (int)(reg_y);
    #endif

    m_registers[reg_x] ^= m_registers[reg_y];

    m_program_counter += 2;
}


// Add the value of register VY to register VX
// Set VF to 01 if a carry occurs
// Set VF to 00 if a carry does not occur
void Computer::inst_8XY4(uint8_t reg_x, uint8_t reg_y)
{
    #ifdef PRINT_OPCODE
    std::cout << "ADD v" << std::hex << (int)(reg_x) << " v" << std::hex << (int)(reg_y);
    #endif

    const uint16_t nx = (uint16_t)m_registers[reg_x];
    const uint16_t ny = (uint16_t)m_registers[reg_y];

    const uint16_t res = nx + ny;

    if (res > 0xFF) {
        m_registers[0xF] = 0x01;
    } else {
        m_registers[0xF] = 0x00;
    }

    m_registers[reg_x] = (res & 0xFF);

    m_program_counter += 2;
}


// Subtract the value of register VY from register VX
// Set VF to 00 if a borrow occurs
// Set VF to 01 if a borrow does not occur
void Computer::inst_8XY5(uint8_t reg_x, uint8_t reg_y)
{
    #ifdef PRINT_OPCODE
    std::cout << "SUB v" << std::hex << (int)(reg_x) << " v" << std::hex << (int)(reg_y);
    #endif

    // TODO: optimize this, there is specific code in C++ for doing this
    const uint8_t nx = m_registers[reg_x];
    const uint8_t ny = m_registers[reg_y];

    if (nx < ny) {
        // a borrow occurs
        m_registers[0xF] = 0x00;
    } else {
        m_registers[0xF] = 0x01;
    }

    m_registers[reg_x] = nx - ny;

    m_program_counter += 2;
}


// Store the value of register VY shifted right one bit in register VX
// Set register VF to the least significant bit prior to the shift
// VY is unchanged
void Computer::inst_8XY6(uint8_t reg_x, uint8_t reg_y)
{
    #ifdef PRINT_OPCODE
    std::cout << "SHFT_R v" << std::hex << (int)(reg_x) << " v" << std::hex << (int)(reg_y);
    #endif

    #ifdef ALT_SHIFT
    const uint8_t v = m_registers[reg_x];
    #else
    const uint8_t v = m_registers[reg_y];
    #endif

    m_registers[reg_x] = v >> 1;
    m_registers[0xF]   = v & 0x01;

    m_program_counter += 2;
}


// Set register VX to the value of VY minus VX
// Set VF to 00 if a borrow occurs
// Set VF to 01 if a borrow does not occur
void Computer::inst_8XY7(uint8_t reg_x, uint8_t reg_y)
{
    #ifdef PRINT_OPCODE
    std::cout << "RSUB v" << std::hex << (int)(reg_x) << " v" << std::hex << (int)(reg_y);
    #endif

    // TODO: optimize this, there is specific code in C++ for doing this
    const uint8_t nx = m_registers[reg_x];
    const uint8_t ny = m_registers[reg_y];

    if (ny < nx) {
        // a borrow occurs
        m_registers[0xF] = 0x00;
    } else {
        m_registers[0xF] = 0x01;
    }

    m_registers[reg_x] = ny - nx;

    m_program_counter += 2;
}


// Store the value of register VY shifted left one bit in register VX
// Set register VF to the most significant bit prior to the shift
// VY is unchanged
void Computer::inst_8XYE(uint8_t reg_x, uint8_t reg_y)
{
    #ifdef PRINT_OPCODE
    std::cout << "SHFT_L v" << std::hex << (int)(reg_x) << " v" << std::hex << (int)(reg_y);
    #endif

    #ifdef ALT_SHIFT
    const uint8_t v = m_registers[reg_x];
    #else
    const uint8_t v = m_registers[reg_y];
    #endif

    m_registers[reg_x] = v << 1;
    m_registers[0xF]   = (v & 0x80) >> 7;

    m_program_counter += 2;
}


// Skip the following instruction if the value of register VX is not equal to
// the value of register VY
void Computer::inst_9XY0(uint8_t reg_x, uint8_t reg_y)
{
    #ifdef PRINT_OPCODE
    std::cout << "SKIP_NEQ v" << std::hex << (int)(reg_x) << " v" << std::hex << (int)(reg_y);
    #endif

    if (m_registers[reg_x] != m_registers[reg_y]) {
        m_program_counter += 4;
    } else {
        m_program_counter += 2;
    }
}


// Store memory address NNN in register I
void Computer::inst_ANNN(uint16_t addr)
{
    #ifdef PRINT_OPCODE
    std::cout << "LOAD_I 0x" << std::hex << addr;
    #endif

    m_I_register = addr;

    m_program_counter += 2;
}


// Jump to address NNN + V0
void Computer::inst_BNNN(uint16_t addr)
{
    #ifdef PRINT_OPCODE
    std::cout << "JMP_V0 0x" << std::hex << addr;
    #endif

    m_program_counter = addr + (uint16_t)m_registers[0x0];
}


// Set VX to a random number with a mask of NN
void Computer::inst_CXNN(uint8_t reg_x, uint8_t mask)
{
    #ifdef PRINT_OPCODE
    std::cout << "RND v" << std::hex << (int)(reg_x) << " 0x" << std::hex << mask;
    #endif

    const float rng_c = (float)std::rand() / (float)(RAND_MAX - 1);
    uint8_t rng = (uint8_t)std::round((float)0xFF * rng_c);
    m_registers[reg_x] = rng & mask;

    m_program_counter += 2;
}


// Draw a sprite at position VX, VY with N bytes of sprite data starting at the
// address stored in I
// Set VF to 01 if any set pixels are changed to unset, and 00 otherwise
void Computer::inst_DXYN(uint8_t reg_x, uint8_t reg_y, uint8_t n_bytes)
{
    #ifdef PRINT_OPCODE
    std::cout << "DRAW v" << std::hex << (int)(reg_x) << " v" << std::hex << (int)(reg_y) << " " << int(n_bytes);
    #endif

    const uint8_t start_x = m_registers[reg_x] % screen_width;
    const uint8_t start_y = m_registers[reg_y] % screen_height;

    // This could be enhanced...
    m_registers[0xF] = 0x00;

    for (uint8_t y = start_y; y < start_y + n_bytes && y < screen_height; y++) {
        const uint8_t mem_val = m_memory[m_I_register + (y - start_y)];

        for (uint8_t b = 0; b < 8; b++) {
            const uint8_t x_b = (start_x + b) / 8;

            if (start_x + b < screen_width) {
                const uint8_t screen_curr_val = m_screen[y * (screen_width/8) + x_b];
                const uint8_t screen_bit_offset = (start_x + b) % 8;
                const uint8_t screen_bit_mask = 1 << (7 - screen_bit_offset);
                const uint8_t screen_curr_b = ((screen_curr_val & screen_bit_mask) != 0);

                const uint8_t mem_bit_offset = b;
                const uint8_t mem_bit_mask = 1 << (7 - mem_bit_offset);
                const uint8_t mem_curr_b = ((mem_val & mem_bit_mask) != 0);
                const uint8_t new_screen_curr_b = screen_curr_b ^ mem_curr_b;

                // TODO:Check if flip from set to unset happend
                if (screen_curr_b && !new_screen_curr_b) {
                    m_registers[0xF] = 0x01;
                }

                if (new_screen_curr_b) {
                    m_screen[y * (screen_width/8) + x_b] |= 1 << (7 - screen_bit_offset);
                }
                else {
                    m_screen[y * (screen_width/8) + x_b] &= ~(1 << (7 - screen_bit_offset));
                }
            }
        }
    }

    m_program_counter += 2;
}


// Skip the following instruction if the key corresponding to the hex value
// currently stored in register VX is pressed
void Computer::inst_EX9E(uint8_t reg_x)
{
    #ifdef PRINT_OPCODE
    std::cout << "JMP_K v" << std::hex << (int)(reg_x);
    #endif

    uint8_t hex_v = m_registers[reg_x];

    if (hex_v <= 0xF && m_keypad[hex_v]) {
        m_program_counter += 4;
    } else {
        m_program_counter += 2;
    }
}


// Skip the following instruction if the key corresponding to the hex value
// currently stored in register VX is not pressed
void Computer::inst_EXA1(uint8_t reg_x)
{
    #ifdef PRINT_OPCODE
    std::cout << "JMP_NO_K v" << std::hex << (int)(reg_x);
    #endif

    uint8_t hex_v = m_registers[reg_x];

    if (hex_v <= 0xF && !m_keypad[hex_v]) {
        m_program_counter += 4;
    } else {
        m_program_counter += 2;
    }
}


// Store the current value of the delay timer in register VX
void Computer::inst_FX07(uint8_t reg_x)
{
    #ifdef PRINT_OPCODE
    std::cout << "STR_DELAY v" << std::hex << (int)(reg_x);
    #endif

    m_registers[reg_x] = m_delay_timer;

    m_program_counter += 2;
}


// Wait for a keypress and store the result in register VX
void Computer::inst_FX0A(uint8_t reg_x)
{
    #ifdef PRINT_OPCODE
    std::cout << "WAIT_K v" << std::hex << (int)(reg_x);
    #endif

    // The instruction has to finish
    if (m_wait_for_key_press) {
        m_wait_for_key_press = false;
        m_registers[reg_x] = m_last_key_pressed;

        m_program_counter += 2;
    } else {
        m_wait_for_key_press = true;
        // We do not advance the program counter,
        // the insctruction will be called again
    }
}


// Set the delay timer to the value of register VX
void Computer::inst_FX15(uint8_t reg_x)
{
    #ifdef PRINT_OPCODE
    std::cout << "DELAY v" << std::hex << (int)(reg_x);
    #endif

    m_delay_timer = m_registers[reg_x];

    m_program_counter += 2;
}


// Set the sound timer to the value of register VX
void Computer::inst_FX18(uint8_t reg_x)
{
    #ifdef PRINT_OPCODE
    std::cout << "SOUND v" << std::hex << (int)(reg_x);
    #endif

    m_sound_timer = m_registers[reg_x];

    m_program_counter += 2;
}


// Add the value stored in register VX to register I
void Computer::inst_FX1E(uint8_t reg_x)
{
    #ifdef PRINT_OPCODE
    std::cout << "ADD_I v" << std::hex << (int)(reg_x);
    #endif

    m_I_register += (uint16_t)m_registers[reg_x];

    m_program_counter += 2;
}


// Set I to the memory address of the sprite data corresponding to the
// hexadecimal digit stored in register VX
void Computer::inst_FX29(uint8_t reg_x)
{
    #ifdef PRINT_OPCODE
    std::cout << "SPRITE 0x" << std::hex << (int)(reg_x);
    #endif

    m_I_register = ((uint16_t)m_registers[reg_x] & 0xF) * 5;

    m_program_counter += 2;
}


// Store the binary-coded decimal equivalent of the value stored in register VX
// at addresses I, I + 1, and I + 2
void Computer::inst_FX33(uint8_t reg_x)
{
    #ifdef PRINT_OPCODE
    std::cout << "CDE v" << std::hex << (int)(reg_x);
    #endif

    const uint8_t vx = m_registers[reg_x];

    const uint8_t a = vx / 100;
    const uint8_t b = (vx - a * 100) / 10;
    const uint8_t c = (vx - a * 100 - b * 10);

    m_memory[m_I_register + 0] = a;
    m_memory[m_I_register + 1] = b;
    m_memory[m_I_register + 2] = c;

    m_program_counter += 2;
}


// Store the values of registers V0 to VX inclusive in memory starting at
// address I
// I is set to I + X + 1 after operation²
void Computer::inst_FX55(uint8_t reg_x)
{
    #ifdef PRINT_OPCODE
    std::cout << "STR_Vn 0x" << std::hex << (int)(reg_x);
    #endif

    // for (uint8_t reg = 0; reg <= reg_x; reg++) {
    //     m_memory[m_I_register + reg] = m_registers[reg];
    // }

    std::memcpy(&m_memory[m_I_register], &m_registers[0], reg_x + 1);

    // Implementation dependent:
    // https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Instruction-Set#notes
    #ifdef ALT_STR_LD
    m_I_register += (uint16_t)reg_x + 1;
    #endif

    m_program_counter += 2;
}


// Fill registers V0 to VX inclusive with the values stored in memory starting
// at address I
// I is set to I + X + 1 after operation²
void Computer::inst_FX65(uint8_t reg_x)
{
    #ifdef PRINT_OPCODE
    std::cout << "LOAD_Vn 0x" << std::hex << (int)(reg_x);
    #endif

    // for (uint8_t reg = 0; reg <= reg_x; reg++) {
    //     m_registers[reg] = m_memory[m_I_register + reg];
    // }

    std::memcpy(&m_registers[0], &m_memory[m_I_register], reg_x + 1);

    // Implementation dependent:
    // https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Instruction-Set#notes
    #ifdef ALT_STR_LD
    m_I_register += (uint16_t)reg_x + 1;
    #endif

    m_program_counter += 2;
}


void Computer::exec(uint16_t instruction)
{
    const uint8_t  reg_x = (instruction & 0x0F00) >> 8;
    const uint8_t  reg_y = (instruction & 0x00F0) >> 4;
    const uint16_t nnn   = instruction & 0x0FFF;
    const uint8_t  nn    = instruction & 0x00FF;
    const uint8_t  n     = instruction & 0x000F;

    // if (instruction != 0x13A8)
    // std::cout << std::hex << instruction << " " << m_program_counter << std::endl;

    // 00E0
    if (instruction == 0x00E0) {
        // Clear the screen
        inst_00E0();
    }
    // 00EE
    else if (instruction == 0x00EE) {
        // Return from a subroutine
        inst_00EE();
    }
    // 0NNN
    else if ((instruction & 0xF000) == 0x0000) {
        // Execute machine language subroutine at address NNN
        inst_0NNN(nnn);
    }
    // 1NNN
    else if ((instruction & 0xF000) == 0x1000) {
        // Jump to address NNN
        inst_1NNN(nnn);
    }
    // 2NNN
    else if ((instruction & 0xF000) == 0x2000) {
        // Execute subroutine starting at address NNN
        inst_2NNN(nnn);
    }
    // 3XNN
    else if ((instruction & 0xF000) == 0x3000) {
        // Skip the following instruction if the value of register VX equals NN
        inst_3XNN(reg_x, nn);
    }
    // 4XNN
    else if ((instruction & 0xF000) == 0x4000) {
        // Skip the following instruction if the value of register VX is not
        // equal to NN
        inst_4XNN(reg_x, nn);
    }
    // 5XY0
    else if ((instruction & 0xF000) == 0x5000) {
        // Skip the following instruction if the value of register VX is equal
        // to the value of register VY
        inst_5XY0(reg_x, reg_y);
    }
    // 6XNN
    else if ((instruction & 0xF000) == 0x6000) {
        // Store number NN in register VX
        inst_6XNN(reg_x, nn);
    }
    // 7XNN
    else if ((instruction & 0xF000) == 0x7000) {
        // Add the value NN to register VX
        inst_7XNN(reg_x, nn);
    }
    // 8XY0
    else if ((instruction & 0xF00F) == 0x8000) {
        // Store the value of register VY in register VX
        inst_8XY0(reg_x, reg_y);
    }
    // 8XY1
    else if ((instruction & 0xF00F) == 0x8001) {
        // Set VX to VX OR VY
        inst_8XY1(reg_x, reg_y);
    }
    // 8XY2
    else if ((instruction & 0xF00F) == 0x8002) {
        // Set VX to VX AND VY
        inst_8XY2(reg_x, reg_y);
    }
    // 8XY3
    else if ((instruction & 0xF00F) == 0x8003) {
        // Set VX to VX XOR VY
        inst_8XY3(reg_x, reg_y);
    }
    // 8XY4
    else if ((instruction & 0xF00F) == 0x8004) {
        // Add the value of register VY to register VX
        // Set VF to 01 if a carry occurs
        // Set VF to 00 if a carry does not occur
        inst_8XY4(reg_x, reg_y);
    }
    // 8XY5
    else if ((instruction & 0xF00F) == 0x8005) {
        // Subtract the value of register VY from register VX
        // Set VF to 00 if a borrow occurs
        // Set VF to 01 if a borrow does not occur
        inst_8XY5(reg_x, reg_y);
    }
    // 8XY6
    else if ((instruction & 0xF00F) == 0x8006) {
        // Store the value of register VY shifted right one bit in register VX¹
        // Set register VF to the least significant bit prior to the shift
        // VY is unchanged
        inst_8XY6(reg_x, reg_y);
    }
    // 8XY7
    else if ((instruction & 0xF00F) == 0x8007) {
        // Set register VX to the value of VY minus VX
        // Set VF to 00 if a borrow occurs
        // Set VF to 01 if a borrow does not occur
        inst_8XY7(reg_x, reg_y);
    }
    // 8XYE
    else if ((instruction & 0xF00F) == 0x800E) {
        // Store the value of register VY shifted left one bit in register VX¹
        // Set register VF to the most significant bit prior to the shift
        // VY is unchanged
        inst_8XYE(reg_x, reg_y);
    }
    // 9XY0
    else if ((instruction & 0xF00F) == 0x9000) {
        // Skip the following instruction if the value of register VX is not
        // equal to the value of register VY
        inst_9XY0(reg_x, reg_y);
    }
    // ANNN
    else if ((instruction & 0xF000) == 0xA000) {
        // Store memory address NNN in register I
        inst_ANNN(nnn);
    }
    // BNNN
    else if ((instruction & 0xF000) == 0xB000) {
        // Jump to address NNN + V0
        inst_BNNN(nnn);
    }
    // CXNN
    else if ((instruction & 0xF000) == 0xC000) {
        // Set VX to a random number with a mask of NN
        inst_CXNN(reg_x, nn);
    }
    // DXYN
    else if ((instruction & 0xF000) == 0xD000) {
        // Draw a sprite at position VX, VY with N bytes of sprite data
        // starting at the address stored in I
        // Set VF to 01 if any set pixels are changed to unset, and 00 otherwise
        inst_DXYN(reg_x, reg_y, n);
    }
    // EX9E
    else if ((instruction & 0xF0FF) == 0xE09E) {
        // Skip the following instruction if the key corresponding to the hex
        // value currently stored in register VX is pressed
        inst_EX9E(reg_x);
    }
    // EXA1
    else if ((instruction & 0xF0FF) == 0xE0A1) {
        // Skip the following instruction if the key corresponding to the hex
        // value currently stored in register VX is not pressed
        inst_EXA1(reg_x);
    }
    // FX07
    else if ((instruction & 0xF0FF) == 0xF007) {
        // Store the current value of the delay timer in register VX
        inst_FX07(reg_x);
    }
    // FX0A
    else if ((instruction & 0xF0FF) == 0xF00A) {
        // Wait for a keypress and store the result in register VX
        inst_FX0A(reg_x);
    }
    // FX15
    else if ((instruction & 0xF0FF) == 0xF015) {
        // Set the delay timer to the value of register VX
        inst_FX15(reg_x);
    }
    // FX18
    else if ((instruction & 0xF0FF) == 0xF018) {
        // Set the sound timer to the value of register VX
        inst_FX18(reg_x);
    }
    // FX1E
    else if ((instruction & 0xF0FF) == 0xF01E) {
        // Add the value stored in register VX to register I
        inst_FX1E(reg_x);
    }
    // FX29
    else if ((instruction & 0xF0FF) == 0xF029) {
        // Set I to the memory address of the sprite data corresponding to the
        // hexadecimal digit stored in register VX
        inst_FX29(reg_x);
    }
    // FX33
    else if ((instruction & 0xF0FF) == 0xF033) {
        // Store the binary-coded decimal equivalent of the value stored in
        // register VX at addresses I, I + 1, and I + 2
        inst_FX33(reg_x);
    }
    // FX55
    else if ((instruction & 0xF0FF) == 0xF055) {
        // Store the values of registers V0 to VX inclusive in memory starting
        // at address I
        // I is set to I + X + 1 after operation²
        inst_FX55(reg_x);
    }
    // FX65
    else if ((instruction & 0xF0FF) == 0xF065) {
        // Fill registers V0 to VX inclusive with the values stored in memory
        // starting at address I
        // I is set to I + X + 1 after operation²
        inst_FX65(reg_x);
    }
    else {
        std::cout << "OpCode? " << std::hex << instruction << std::endl;
        m_program_counter += 2;
    }

    #ifdef PRINT_OPCODE
    std::cout << std::endl;
    #endif
}