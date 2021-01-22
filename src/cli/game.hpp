#pragma once

#include "boot.hpp"

#include <cstdint>
#include <string>

namespace rge
{
    class Game
    {
    private:
        uint64_t m_last_update_time = 0, m_last_fps_counter_reset_time = 0;
        uint32_t m_fps_counter = 0, m_fps = 0;

    public:
    	std::string title;
        bool m_should_close = false;
        uint32_t m_width, m_height;

    protected:
        virtual void on_init() {}
        virtual void on_update(double dt) {}
        virtual void on_render() {}
        virtual void on_shutdown() {}

		virtual void on_sdl_event(SDL_Event& event) {}

        virtual void on_resize(uint32_t width, uint32_t height) {}

    public:
        void init();
        void update();
        void render();
        void shutdown();

        void resize(uint32_t width, uint32_t height);

		void notify_sdl_event(SDL_Event& event);
    };


}