#pragma once

#include "../Tank.h"

class EmscriptenTankRenderer
{
public:
    static void init(const char* url = "sprites.png");
    static bool spritesReady();
    static void render(const Tank& tank);
};
