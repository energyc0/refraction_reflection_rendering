#include "Timer.h"

Timer::Timer() {
    startTimePoint = lastTimePoint = std::chrono::high_resolution_clock::now();
}

float Timer::getDeltaTime() {
    auto temp = lastTimePoint;
    lastTimePoint = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float, std::chrono::milliseconds::period>(lastTimePoint - temp).count();
}

float Timer::getProgramTime() {
    return std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTimePoint).count();
}