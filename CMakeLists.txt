cmake_minimum_required(VERSION 3.21)
set(CMAKE_C_STANDARD 11)

project(chip8)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
    renderer.h  renderer.c
    keyboard.h  keyboard.c
    speaker.h   speaker.c
    cpu.h       cpu.c
    chip8.h     chip8.c
    utils/string.h
    utils/map.h
    utils/stack.h
    utils/type_alias.h
    main.c
)
target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    m
)