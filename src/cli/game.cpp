#include "game.hpp"

#include <cstdio>

#include <bgfx/bgfx.h>

using namespace rge;

void Game::init()
{
	on_init();
}

void Game::update()
{
	Uint64 now = SDL_GetPerformanceCounter();
	Uint64 f = SDL_GetPerformanceFrequency();

	double dt = double(now - m_last_update_time) / (double)f;
	m_last_update_time = now;

	double fps_dt = double(now - m_last_fps_counter_reset_time) / (double)f;
	if (fps_dt >= 1.0) {
		m_fps = m_fps_counter;
		printf("FPS: %d\n", m_fps);

		m_fps_counter = 0;
		m_last_fps_counter_reset_time = now;
	} else {
		m_fps_counter++;
	}

	on_update(dt);
}

void Game::render()
{
	on_render();
}

void Game::shutdown()
{
	on_shutdown();
}

void Game::resize(uint32_t width, uint32_t height)
{
	on_resize(width, height);
}

