//
// Created by Om Patil on 19/06/25.
//

#pragma once
#include "Timer.h"

class Animation {
    Timer timer;
    int frameCount;

public:
    Animation() : timer(0), frameCount(0) {}
    Animation(int frameCount, float length) : frameCount(frameCount), timer(length) {}

    float getLength() const {
        return timer.getLength();
    }
    int currentFrame() const {
        return static_cast<int>(timer.getTime()/timer.getLength() * frameCount);
    }

    void step(float deltaTime) {
        timer.step(deltaTime);
    }
};

