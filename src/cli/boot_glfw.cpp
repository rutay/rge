
#if defined(BUILD_CLIENT) && defined(USE_GLFW)

#include "boot.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <bx/timer.h>
#include <bx/math.h>
#include <bx/uint32_t.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

#include <bgfx/platform.h>

#include <fstream>
#include <iostream>

#include "game.hpp"

#include "cli/render/camera.hpp"

static void* glfwNativeWindowHandle(GLFWwindow* _window)
{
    return glfwGetWin32Window(_window);
}

static void glfwSetWindow(GLFWwindow* _window)
{
    bgfx::PlatformData pd;
    pd.ndt = NULL;
    pd.nwh = glfwNativeWindowHandle(_window);
    pd.context = NULL;
    pd.backBuffer = NULL;
    pd.backBufferDS = NULL;
    bgfx::setPlatformData(pd);
}

int rge::boot(Game& game, int argc, char* argv[])
{
    if (!glfwInit()) {
        std::cerr << "Couldn't init GLFW." << std::endl;
        exit(1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    uint32_t width = 500, height = 500;

    GLFWwindow* window = glfwCreateWindow(width, height, "Game", NULL, NULL);

    if (!window) {
        std::cerr << "GLFW window is NULL." << std::endl;
        exit(1);
    }

    glfwSetWindow(window);

    // Set view 0 clear state.

    auto time_offset = bx::getHPCounter();

    // ---------------------------------------  my main

    game.init();

    // --------------------------------------- camera

    Camera camera;

    float const camera_speed = 50.0f; // m/s
    float const camera_rotation_speed = 0.001; // rad/pixel

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double last_cursor_x = -1, last_cursor_y = -1;

    // --------------------------------------- looping

    bgfx::frame();

    double last_time = -1;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE))
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        // Updating
        double current_time = glfwGetTime();
        double dt = 0;
        if (last_time != -1) {
            dt = current_time - last_time;
        }

        game.update(dt);

        last_time = current_time;

        // camera controller

        if (glfwGetKey(window, GLFW_KEY_W)) camera.offset_position(camera.forward(), camera_speed * dt);
        if (glfwGetKey(window, GLFW_KEY_S)) camera.offset_position(camera.forward(), -camera_speed * dt);
        if (glfwGetKey(window, GLFW_KEY_A)) camera.offset_position(camera.right(), camera_speed * dt);
        if (glfwGetKey(window, GLFW_KEY_D)) camera.offset_position(camera.right(), -camera_speed * dt);
        if (glfwGetKey(window, GLFW_KEY_SPACE))      camera.offset_position(bx::Vec3{0, 1, 0}, camera_speed * dt);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) camera.offset_position(bx::Vec3{0, 1, 0}, -camera_speed * dt);

        {
            double cursor_x, cursor_y;
            glfwGetCursorPos(window, &cursor_x, &cursor_y);

            float dx = cursor_x - last_cursor_x;
            float dy = last_cursor_y - cursor_y;

            float d_yaw = dx * camera_rotation_speed;
            float d_pitch = dy * camera_rotation_speed;

            camera.offset_orientation(d_yaw, d_pitch);

            last_cursor_x = cursor_x;
            last_cursor_y = cursor_y;
        }

        float camera_view_mat[16], camera_projection_mat[16];

        camera.view(camera_view_mat);
        camera.projection(camera_projection_mat);

        bgfx::setViewTransform(0, camera_view_mat, camera_projection_mat);

        // Rendering
        // Updates the framebuffer size in case the window has resized.
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        camera.m_aspect_ratio = width / static_cast<float>(height);

        bgfx::setViewRect(0, 0, 0, width, height);

        bgfx::touch(0); // This call ensures that view 0 is cleared if no draw calls are submitted.

        // Here I can do my dirty, but cutie (!!!), rendering stuff:

        game.render();

        bgfx::frame(); // Advances to the next frame.

        //glfwSwapBuffers(window);
    }

    game.shutdown();

    glfwDestroyWindow(window);

    glfwTerminate();
}

#endif
