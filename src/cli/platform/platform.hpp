#pragma once

#include <SDL.h>

namespace rge
{
    class Game;

    int boot(Game& game, int argc, char* argv[]);
}

template<typename T>
int __main(int argc, char* argv[])
{
	T game;
	return rge::boot(game, argc, argv);
}

#define RGE_define_game(Game) int main(int argc, char* argv[]) { return __main<Game>(argc, argv); };
