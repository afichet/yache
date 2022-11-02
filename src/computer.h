#pragma once

#include <array>
#include <vector>
#include <cstdint>

#define ALT_SHIFT
// #define ALT_STR_LD

// #define PRINT_OPCODE

class Computer
{
public:
    Computer(const std::vector<uint8_t> &program);

    void keyPress(uint8_t key);
    void keyRelease(uint8_t key);

    void tick();

    uint8_t width()  const { return screen_width; }
    uint8_t height() const { return screen_height; }

    uint16_t delayTimer() const { return m_delay_timer; }
    uint16_t soundTimer() const { return m_sound_timer; }

    const std::vector<uint8_t>& screen() const { return m_screen; }

protected:
    void exec(uint16_t inst);

    void inst_0NNN(uint16_t addr);
    void inst_00E0();
    void inst_00EE();
    void inst_1NNN(uint16_t addr);
    void inst_2NNN(uint16_t addr);
    void inst_3XNN(uint8_t reg_x, uint8_t value);
    void inst_4XNN(uint8_t reg_x, uint8_t value);
    void inst_5XY0(uint8_t reg_x, uint8_t reg_y);
    void inst_6XNN(uint8_t reg_x, uint8_t value);
    void inst_7XNN(uint8_t reg_x, uint8_t value);
    void inst_8XY0(uint8_t reg_x, uint8_t reg_y);
    void inst_8XY1(uint8_t reg_x, uint8_t reg_y);
    void inst_8XY2(uint8_t reg_x, uint8_t reg_y);
    void inst_8XY3(uint8_t reg_x, uint8_t reg_y);
    void inst_8XY4(uint8_t reg_x, uint8_t reg_y);
    void inst_8XY5(uint8_t reg_x, uint8_t reg_y);
    void inst_8XY6(uint8_t reg_x, uint8_t reg_y);
    void inst_8XY7(uint8_t reg_x, uint8_t reg_y);
    void inst_8XYE(uint8_t reg_x, uint8_t reg_y);
    void inst_9XY0(uint8_t reg_x, uint8_t reg_y);
    void inst_ANNN(uint16_t addr);
    void inst_BNNN(uint16_t addr);
    void inst_CXNN(uint8_t reg_x, uint8_t mask);
    void inst_DXYN(uint8_t reg_x, uint8_t reg_y, uint8_t n_bytes);
    void inst_EX9E(uint8_t reg_x);
    void inst_EXA1(uint8_t reg_x);
    void inst_FX07(uint8_t reg_x);
    void inst_FX0A(uint8_t reg_x);
    void inst_FX15(uint8_t reg_x);
    void inst_FX18(uint8_t reg_x);
    void inst_FX1E(uint8_t reg_x);
    void inst_FX29(uint8_t reg_x);
    void inst_FX33(uint8_t reg_x);
    void inst_FX55(uint8_t reg_x);
    void inst_FX65(uint8_t reg_x);

protected:
    std::array<uint8_t, 16> m_registers;
    
    bool m_wait_for_key_press;
    uint8_t m_last_key_pressed;

    uint16_t m_delay_timer;
    uint16_t m_sound_timer;
    uint16_t m_I_register;
    uint16_t m_program_counter;
    std::vector<uint16_t> m_stack;
    std::array<uint8_t, 0xFFFF> m_memory;

    // std::uint16_t m_stack_pointer;
    static constexpr uint8_t screen_width  = 0x3F + 1;
    static constexpr uint8_t screen_height = 0x1F + 1;

    static constexpr uint8_t char_0[5] = {0xF0, 0x90, 0x90, 0x90, 0xF0};
    static constexpr uint8_t char_1[5] = {0x20, 0x60, 0x20, 0x20, 0x70};
    static constexpr uint8_t char_2[5] = {0xF0, 0x10, 0xF0, 0x80, 0xF0};
    static constexpr uint8_t char_3[5] = {0xF0, 0x10, 0xF0, 0x10, 0xF0};
    static constexpr uint8_t char_4[5] = {0x90, 0x90, 0xF0, 0x10, 0x10};
    static constexpr uint8_t char_5[5] = {0xF0, 0x80, 0xF0, 0x10, 0xF0};
    static constexpr uint8_t char_6[5] = {0xF0, 0x80, 0xF0, 0x90, 0xF0};
    static constexpr uint8_t char_7[5] = {0xF0, 0x10, 0x20, 0x40, 0x40};
    static constexpr uint8_t char_8[5] = {0xF0, 0x90, 0xF0, 0x90, 0xF0};
    static constexpr uint8_t char_9[5] = {0xF0, 0x90, 0xF0, 0x10, 0xF0};
    static constexpr uint8_t char_A[5] = {0xF0, 0x90, 0xF0, 0x90, 0x90};
    static constexpr uint8_t char_B[5] = {0xE0, 0x90, 0xE0, 0x90, 0xE0};
    static constexpr uint8_t char_C[5] = {0xF0, 0x80, 0x80, 0x80, 0xF0};
    static constexpr uint8_t char_D[5] = {0xE0, 0x90, 0x90, 0x90, 0xE0};
    static constexpr uint8_t char_E[5] = {0xF0, 0x80, 0xF0, 0x80, 0xF0};
    static constexpr uint8_t char_F[5] = {0xF0, 0x80, 0xF0, 0x80, 0x80};

    // std::array<uint8_t, screen_width * screen_height> m_screen;
    std::vector<uint8_t> m_screen;
    std::array<bool, 16> m_keypad;
};