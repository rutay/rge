#include "entry.hpp"

#include <cstdio>

void Simple_Game::on_init()
{
    printf("Init\n");

    //rge::GltfSceneLoader loader;
    //m_scene = loader.load_from_resource("assets/models/McLaren.glb");
    //m_renderer = new rge::Renderer();
}

void Simple_Game::on_sdl_event(SDL_Event &event)
{
	//printf("SDL event: %d\n", event.type);
}

void Simple_Game::on_render()
{
    //printf("Rendering\n");
    //m_renderer->render(0, m_scene, m_camera);
}

RGE_DEFINE_MAIN(Simple_Game)
