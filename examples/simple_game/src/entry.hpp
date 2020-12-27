#pragma once

#include "cli/game.hpp"
#include "scene/scene.hpp"

class Simple_Game : public Game
{
private:
    rge::Node* m_scene;
    
public:
    void on_init();
    void on_render();
};
