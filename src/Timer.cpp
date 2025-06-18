//
// Created by Om Patil on 18/06/25.
//

#include "Timer.h"

#include <ctime>
#include <__locale>

Timer::Timer(float length) {
    this->length = length;
    time = 0;
    timeout = false;
}

void Timer::step(float deltaTime) {
    time += deltaTime;
    if (time >= length) {
        time -= length;
        timeout = true;
    }
}

bool Timer::isTimeout() const {
    return timeout;
}
float Timer::getLength() const {
    return length;
}
float Timer::getTime() const {
    return time;
}

void Timer::reset() {
    time = 0;
}
