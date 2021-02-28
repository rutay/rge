#include "game.hpp"

#include "cli/renderer/material_program.hpp"

#include <cstdio>

#include <bgfx/bgfx.h>

using namespace rge;

void Game::init()
{
	bgfx::Init init;
 	init.type = bgfx::RendererType::OpenGL;
	init.vendorId = BGFX_PCI_ID_NONE;
	init.resolution.width = m_width;
	init.resolution.height = m_height;
	init.resolution.reset = BGFX_RESET_VSYNC;

	bgfx::init(init);

	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x0000ff00, 1.0f, 0);

	rge::renderer::MaterialProgramRegistry::init();

	auto render_type = bgfx::getRendererType();

	on_init();
}

void Game::notify_sdl_event(SDL_Event &event)
{
	if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
		m_should_close = true;
	}
	on_sdl_event(event);
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
	bgfx::touch(0); // Dummy call to make sure clear happens if no draw call is submitted

	bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

	on_render();

	bgfx::frame();
}

void Game::shutdown()
{
	on_shutdown();
}

// ------------------------------------------------------------------------------------------------
// Screen
// ------------------------------------------------------------------------------------------------

void Game::resize(uint32_t width, uint32_t height)
{
	on_resize(width, height);
}

