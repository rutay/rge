#include "platform.hpp"

#include <cstdio>

#include <SDL.h>
#include <SDL_syswm.h>
#include <glad/glad.h>

#include "cli/game.hpp"
#include "resource_def.hpp"

using namespace rge;

int rge::boot(Game &game, int argc, char *argv[])
{
	rge::resources::init();

	SDL_Init(0);

	// TODO read game title / default width / height from somewhere before initializing

	game.m_title = "RGE Game";
	game.m_width = 512;
	game.m_height = 512;

	SDL_Window *window = SDL_CreateWindow(game.m_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, game.m_width, game.m_height, SDL_WINDOW_OPENGL);

	if (window == nullptr) {
		printf("Couldn't create SDL2 window");
		return 1;
	}

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);

	if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
		printf("Couldn't initialize GLAD");
		exit(1);
	}

	game.init();

	while (!game.m_should_close)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			game.on_event(event);
		}

		game.update();

		game.render();

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
