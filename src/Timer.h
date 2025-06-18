//
// Created by Om Patil on 18/06/25.
//

#pragma once

class Timer {
    float length, time;
    bool timeout;

    public:
    explicit Timer(float length);

    void step(float deltaTime);
    bool isTimeout() const;
    float getLength() const;
    float getTime() const;
    void reset();
};


