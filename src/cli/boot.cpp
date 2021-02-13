#include "boot.hpp"

#include <cstdio>

#include <SDL.h>
#include <SDL_syswm.h>

#include <bgfx/platform.h>

#include "game.hpp"

using namespace rge;

// Reference (SDL2 + BGFX):
// https://github.com/bkaradzic/bgfx/blob/master/examples/common/entry/entry_sdl.cpp

static void *sdlNativeWindowHandle(SDL_Window *_window)
{
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version)
	if (!SDL_GetWindowWMInfo(_window, &wmi)) {
		return nullptr;
	}

#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#		if ENTRY_CONFIG_USE_WAYLAND
	wl_egl_window *win_impl = (wl_egl_window*)SDL_GetWindowData(_window, "wl_egl_window");
	if(!win_impl)
	{
		int width, height;
		SDL_GetWindowSize(_window, &width, &height);
		struct wl_surface* surface = wmi.info.wl.surface;
		if(!surface)
			return nullptr;
		win_impl = wl_egl_window_create(surface, width, height);
		SDL_SetWindowData(_window, "wl_egl_window", win_impl);
	}
	return (void*)(uintptr_t)win_impl;
#		else
	return (void*)wmi.info.x11.window;
#		endif
#	elif BX_PLATFORM_OSX
	return wmi.info.cocoa.window;
#	elif BX_PLATFORM_WINDOWS
	return wmi.info.win.window;
#	endif // BX_PLATFORM_
}

inline bool bgfx_setPlatformData(SDL_Window *_window)
{
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version)
	if (!SDL_GetWindowWMInfo(_window, &wmi)) {
		return false;
	}

	bgfx::PlatformData pd;
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#		if ENTRY_CONFIG_USE_WAYLAND
	pd.ndt          = wmi.info.wl.display;
#		else
	pd.ndt          = wmi.info.x11.display;
#		endif
#	elif BX_PLATFORM_OSX
	pd.ndt          = NULL;
#	elif BX_PLATFORM_WINDOWS
	pd.ndt = nullptr;
#	endif // BX_PLATFORM_
	pd.nwh = sdlNativeWindowHandle(_window);

	pd.context = nullptr;
	pd.backBuffer = nullptr;
	pd.backBufferDS = nullptr;
	bgfx::setPlatformData(pd);

	return true;
}

int rge::boot(Game &game, int argc, char *argv[])
{
	SDL_Init(0);

	// TODO read game title / default width / height from somewhere before initializing

	game.title    = "RGE Game";
	game.m_width  = 512;
	game.m_height = 512;

	SDL_Window *window = SDL_CreateWindow(
		game.title.c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		game.m_width, game.m_height,
		NULL
		);

	bgfx_setPlatformData(window);

	game.init();

	if (window == nullptr) {
		printf("Couldn't create SDL2 window");
		return 1;
	}

	bgfx::frame();

	while (!game.m_should_close) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			game.notify_sdl_event(event);
		}
		game.update();
		game.render();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
