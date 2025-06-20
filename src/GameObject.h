//
// Created by Om Patil on 19/06/25.
//

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Animation.h"
#include <SDL3/SDL.h>

enum class ObjectType {
    player, level, enemy
};

struct GameObject {
    ObjectType type;
    glm::vec2 position, velocity, acceleration;
    int direction;
    std::vector<Animation> animations;
    int currentAnimation;
    SDL_Texture *texture;

    GameObject() {
        type = ObjectType::level;
        direction = 1;
        position = velocity = acceleration = glm::vec2(0);
        currentAnimation = -1;
        texture = nullptr;
    }
};
