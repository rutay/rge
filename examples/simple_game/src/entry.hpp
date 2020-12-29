#pragma once

#include "cli/game.hpp"
#include "scene/scene.hpp"

#include "cli/render/renderer.hpp"
#include "cli/render/camera.hpp"

class Simple_Game : public Game
{
private:
    rge::Node* m_scene;
    rge::Renderer* m_renderer;
    rge::Camera m_camera;

public:
    void on_init();
    void on_render();
};
