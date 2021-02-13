#include "entry.hpp"

#include <cstdio>

#include "scene/gltf_scene_loader.hpp"

void Simple_Game::on_init()
{
    printf("Init\n");

    rge::GltfSceneLoader loader;
    m_scene = loader.load_from_resource("assets/models/McLaren.glb");

    m_renderer = new rge::Renderer();

	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void Simple_Game::on_sdl_event(SDL_Event &event)
{
	m_camera_movement_ctrl.on_sdl_event(event);
	m_camera_orientation_ctrl.on_sdl_event(m_camera, event);
}

void Simple_Game::on_update(double dt)
{
	printf("position x=%f y=%f z=%f | yaw=%f pitch=%f\n",
		m_camera.m_position.x, m_camera.m_position.y, m_camera.m_position.z,
		m_camera.m_yaw, m_camera.m_pitch
		);
	m_camera_movement_ctrl.update(m_camera, dt);
}

void Simple_Game::on_render()
{
    //printf("Rendering\n");
    m_renderer->render(0, m_scene, m_camera);
}

RGE_DEFINE_MAIN(Simple_Game)
