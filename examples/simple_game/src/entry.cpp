#include "entry.hpp"

#include <stdio.h>

#include "cli/boot.hpp"

#include "scene/gltf_scene_loader.hpp"

void Simple_Game::on_init()
{
    printf("Init\n");

    rge::GltfSceneLoader loader;
    m_scene = loader.load_from_resource("assets/models/McLaren.glb");

    m_renderer = new rge::Renderer();
}

void Simple_Game::on_render()
{
    m_renderer->render(0, m_scene, &m_camera);
}

RGE_DEFINE_MAIN(Simple_Game);
