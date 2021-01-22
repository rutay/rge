#pragma once

#include "cli/game.hpp"

class Simple_Game : public rge::Game
{
private:
    //rge::Node* m_scene;
    //rge::Renderer* m_renderer;
    //rge::Camera m_camera;

public:
    void on_init() override;
    void on_sdl_event(SDL_Event& event) override;
    void on_render() override;
};
