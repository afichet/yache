# Yet Another CHIP-8 Emulator
project(Yache)

cmake_minimum_required(VERSION 3.1)

add_executable(yache
    src/main.cpp
    src/computer.cpp
    src/beeper.cpp
)

target_include_directories(yache PRIVATE src/)

find_package(SDL2)

target_link_libraries(yache PRIVATE SDL2::SDL2)