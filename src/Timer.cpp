#include "Timer.h"
#include <iostream>

Timer::Timer(float show_FPS_delay_ms) : showDelay_ms(show_FPS_delay_ms){
    if (showDelay_ms <= 0.0f) {
        std::cerr << "Incorrect timer delay!";
        exit(EXIT_FAILURE);
    }
    startTimePoint = lastTimePoint = std::chrono::high_resolution_clock::now();
    deltaTime = 0.0f;
    timePast_ms = showDelay_ms;
}
void Timer::Tick() {
    auto temp = lastTimePoint;
    lastTimePoint = std::chrono::high_resolution_clock::now();
    float delta = std::chrono::duration<float, std::chrono::milliseconds::period>(lastTimePoint - temp).count();
    timePast_ms += delta;
    if (timePast_ms >= showDelay_ms) {
        deltaTime = delta;
        timePast_ms = 0.0f;
    }
}