#if defined(__EMSCRIPTEN__)

#include "boot.hpp"

// TODO

#include <emscripten.h>
#include <emscripten/html5.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

void update(void* user_data)
{
    auto app = static_cast<App*>(user_data);
    app->update(1.0f); // TODO
    app->render();
}

int main()
{
    char const* canvas = "#canvas";

    bgfx::PlatformData pd;
    pd.ndt = NULL;
    pd.nwh = (void*)canvas;
    pd.context = NULL;
    pd.backBuffer = NULL;
    pd.backBufferDS = NULL;
    bgfx::setPlatformData(pd);
    
    uint32_t canvas_width  = 500;
    uint32_t canvas_height = 500;

    App app;
    app.m_width  = canvas_width;
    app.m_height = canvas_height;
    app.init();

    bgfx::frame();

    emscripten_set_main_loop_arg(&update, &app, -1, true);

    app.shutdown();

    return 0;
}

#endif
