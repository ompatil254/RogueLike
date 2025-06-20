//
// Created by Om Patil on 19/06/25.
//

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Animation.h"
#include <SDL3/SDL.h>

enum class PlayerState {
    idle, running, jumping
};

struct PlayerData {
    PlayerState state;
    PlayerData() {
        state = PlayerState::idle;
    }
};

struct LevelData {};
struct EnemyData {};

union ObjectData {
    PlayerData player;
    LevelData level;
    EnemyData enemy;
};


enum class ObjectType {
    player, level, enemy
};

struct GameObject {
    ObjectType type;
    ObjectData data;
    glm::vec2 position, velocity, acceleration;
    std::vector<Animation> animations;
    SDL_Texture *texture;
    int direction;
    int currentAnimation;
    float maxSpeedX;


    GameObject() :data{.level = LevelData()} {
        type = ObjectType::level;
        maxSpeedX = 0;
        direction = 1;
        position = velocity = acceleration = glm::vec2(0);
        currentAnimation = -1;
        texture = nullptr;
    }
};
