#pragma once

#include <cli/render/renderer.hpp>
#include "cli/game.hpp"

#include "scene/scene.hpp"

class Simple_Game : public rge::Game
{
private:
    rge::Node* m_scene;
    rge::Renderer* m_renderer;

    rge::Camera m_camera;
    rge::Camera_Movement_KeyboardController m_camera_movement_ctrl;
	rge::Camera_Orientation_MouseController m_camera_orientation_ctrl;

public:
    void on_init() override;
    void on_sdl_event(SDL_Event& event) override;
    void on_update(double dt) override;
    void on_render() override;
};
