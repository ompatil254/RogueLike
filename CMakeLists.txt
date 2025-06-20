cmake_minimum_required(VERSION 3.31)
project(roguelike)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(glm REQUIRED)

set(CMAKE_CXX_STANDARD 20)

add_executable(roguelike src/main.cpp
        src/timer.cpp
        src/timer.h
        src/Animation.h
        src/GameObject.h)

target_link_libraries(roguelike PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL2_mixer::SDL2_mixer glm::glm)