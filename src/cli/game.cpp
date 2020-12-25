#include "game.hpp"

#include <stdlib.h>
#include <stdio.h>

#include <bgfx/bgfx.h>

void Game::init()
{
    bgfx::Init init;
    init.type = bgfx::RendererType::Count;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.resolution.width = m_width;
    init.resolution.height = m_height;
    init.resolution.reset = BGFX_RESET_VSYNC;

    bgfx::init(init);

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x0000ffff, 1.0f, 0);

    on_init();
}

void Game::update(float dt)
{
    on_update(dt);
}

void Game::render()
{
    bgfx::touch(0);

    bgfx::setViewRect(0, 0, 0, m_width, m_height);

    on_render();

    bgfx::frame();
}

void Game::shutdown()
{
    on_shutdown();
}
