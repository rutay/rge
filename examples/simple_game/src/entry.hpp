#pragma once

#include "cli/renderer/renderer.hpp"
#include "cli/game.hpp"

#include "scene/scene.hpp"

class MyGame : public rge::Game
{
private:
    rge::Node* m_scene;
    rge::Renderer* m_renderer;

    rge::FreeCamera m_free_camera;
    rge::FreeCamera_KeyboardMovementController m_camera_movement_ctrl;
	rge::FreeCamera_OrientationMouseController m_camera_orientation_ctrl;

public:
    void on_init() override;
    void on_event(SDL_Event& event) override;
    void on_update(double dt) override;
    void on_render() override;
};
