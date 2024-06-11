#pragma once
#include <chrono>

class Timer {
    std::chrono::steady_clock::time_point lastTimePoint;
    std::chrono::steady_clock::time_point startTimePoint;
public:
    Timer();
    float getDeltaTime();
    float getProgramTime();
};