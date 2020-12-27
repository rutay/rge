#include "entry.hpp"

#include <stdio.h>

#include "cli/boot.hpp"

#include "scene/gltf_scene_loader.hpp"

void Simple_Game::on_init()
{
    printf("Init\n");

    rge::GltfSceneLoader loader;
    m_scene = loader.load_from_resource("assets/models/gumball_darwin.glb");
}

void Simple_Game::on_render()
{
}

RGE_DEFINE_MAIN(Simple_Game);
