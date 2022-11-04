# Yet Another CHIP-8 Emulator
project(Yache)

cmake_minimum_required(VERSION 3.1)

add_executable(yache
    src/main.cpp
    src/computer.cpp
    src/beeper.cpp
)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

target_include_directories(yache PRIVATE src/)

find_package(SDL2)

target_link_libraries(yache PRIVATE SDL2::SDL2)

if (WIN32)
    target_link_libraries(yache PRIVATE SDL2::SDL2main)
endif()

