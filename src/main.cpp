#include "App.h"

int main()
{
    constexpr int width = 800,
        height = 600;
    const char* applicationName = "DEMO 01";
    const char* engineName = "None";
    App app(width, height, applicationName, engineName);
    app.run();
}

