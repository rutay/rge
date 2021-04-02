#pragma once

#include <SDL.h>

#include <cstdint>
#include <string>

namespace rge
{

// ------------------------------------------------------------------------------------------------
// Game
// ------------------------------------------------------------------------------------------------

class Game
{
private:
	uint64_t m_last_update_time = 0, m_last_fps_counter_reset_time = 0;
	uint32_t m_fps_counter = 0, m_fps = 0;

public:
	std::string m_title;
	bool m_should_close = false;
	uint32_t m_width, m_height;

protected:
	virtual void on_init() {}
	virtual void on_update(double dt) {}
	virtual void on_render() {}
	virtual void on_shutdown() {}

	virtual void on_resize(uint32_t width, uint32_t height) {}

public:
	void init();
	void update();
	void render();
	void shutdown();
	virtual void on_event(SDL_Event& event) {}
	void resize(uint32_t width, uint32_t height);
};

// ------------------------------------------------------------------------------------------------
// Boot
// ------------------------------------------------------------------------------------------------



}