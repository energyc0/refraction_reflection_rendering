#pragma once
#include <chrono>

class Timer {
    const float showDelay_ms;
    float deltaTime;
    float timePast_ms;
    std::chrono::steady_clock::time_point lastTimePoint;
    std::chrono::steady_clock::time_point startTimePoint;
public:
    Timer(float show_FPS_delay);
    inline float getDeltaTime() const {
        return deltaTime;
    }
    inline float getProgramTime() const {
        return std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTimePoint).count();
    }
    void Tick();
};