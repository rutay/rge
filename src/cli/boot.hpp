#pragma once

#include <SDL.h>

namespace rge
{
    class Game;

    int boot(Game& game, int argc, char* argv[]);
}

#define RGE_DEFINE_MAIN(Game)               \
    int main(int argc, char* argv[])    \
    {                                       \
        Game game;                          \
        return rge::boot(game, argc, argv); \
    }

