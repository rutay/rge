#include "platform.hpp"

#include <cstdio>

#include <SDL.h>
#include <glad/glad.h>

#include "cli/game.hpp"
#include "rge_init.hpp"

using namespace rge;

void GLAPIENTRY gl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    fprintf( stdout, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
             ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
             type, severity, message );
}

int rge::boot(Game &game, int argc, char *argv[])
{
	rge::init();

    if (SDL_Init(0) != 0)
    {
        printf("Unable to initialize SDL2: %s", SDL_GetError());
        exit(1);
    }

	// TODO read game title / default width / height from somewhere before initializing

	game.m_title = "RGE Game";
	game.m_width = 512;
	game.m_height = 512;

	SDL_Window *window = SDL_CreateWindow(
	        game.m_title.c_str(),
	        SDL_WINDOWPOS_CENTERED,
	        SDL_WINDOWPOS_CENTERED,
            (int) game.m_width,
            (int) game.m_height,
	        SDL_WINDOW_OPENGL
	        );

	if (window == nullptr) {
		printf("Couldn't create SDL2 window");
		exit(1);
	}

    // OpenGL 3.2 profile for compatibility with WebGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4); // to have access to debugging features
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        printf("Couldn't initialize GLAD");
        exit(2);
    }

    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    printf("Shading:  %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Use v-sync
    SDL_GL_SetSwapInterval(1);

    // Debug handler
    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback(gl_debug_message_callback, 0);

    game.init();

	while (!game.m_should_close)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
		    switch (event.type)
            {
                case SDL_QUIT:
                    game.m_should_close = true;
                    break;
            }

			game.on_event(event);
		}

		game.update();
		game.render();

        SDL_GL_SwapWindow(window);
        SDL_Delay(1);
	}

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
