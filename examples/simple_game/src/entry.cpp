#include "entry.hpp"

#include <cstdlib>
#include <cstdio>

#include "scene/tinygltf_scene_loader.hpp"
#include "scene/utils.hpp"
#include "scene/material.hpp"

using namespace rge::scene;

void MyGame::on_init()
{
    printf("Init\n");

    rge::scene::SceneLoader_tinygltf loader;
    //m_scene = loader.load_from_resource("assets/models/McLaren.glb");

	m_scene = new Node();

	Mesh* mesh = new Mesh();
	mesh->m_geometry = &rge::scene::utils::CubeGeometry;

	BasicMaterial* material = new BasicMaterial;
	material->m_color[0] = 1.0f;
	material->m_color[3] = 1.0f;
	mesh->m_material = material;

	m_scene->m_meshes.push_back(mesh);

	m_renderer = new rge::renderer::bgfxRenderer();

	SDL_SetRelativeMouseMode(SDL_TRUE);

	m_free_camera.m_aspect_ratio = float(m_width) / m_height;
}

void MyGame::on_sdl_event(SDL_Event &event)
{
	m_camera_movement_ctrl.on_sdl_event(event);
	m_camera_orientation_ctrl.on_sdl_event(m_free_camera, event);
}

void MyGame::on_update(double dt)
{
	/*
	printf("position x=%f y=%f z=%f | yaw=%f pitch=%f\n",
		m_camera.m_position.x, m_camera.m_position.y, m_camera.m_position.z,
		m_camera.m_yaw, m_camera.m_pitch
		);
	 */
	m_camera_movement_ctrl.update(m_free_camera, dt);
}

void MyGame::on_render()
{
    //printf("Rendering\n");
    m_renderer->render(0, m_scene, m_free_camera);
}

RGE_DEFINE_MAIN(MyGame)
