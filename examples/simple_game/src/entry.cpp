#include "entry.hpp"

#include <cstdio>

#include "scene/tinygltf_scene_loader.hpp"
#include "scene/utils.hpp"
#include "scene/material.hpp"
#include "scene/materials_def.hpp"

#include "cli/platform/platform.hpp"

using namespace rge;

Node* create_cube(rge::Vec3 position, rge::Vec4 color)
{
	Node* node = new Node();
	node->m_position = position;

	Mesh* mesh = new Mesh();
	mesh->m_geometry = &rge::utils::CubeGeometry;

	node->m_meshes.push_back(mesh);

	materials::BasicMaterial* material = new materials::BasicMaterial();
	material->m_color = color;
	mesh->m_material = material;

	return node;
}

void MyGame::on_init()
{
    printf("Init\n");

    rge::SceneLoader_tinygltf loader;

    m_scene = new Node();

    Node* gumball_darwin = loader.load_from_resource(resources::Model::GumballDarwin);
	m_scene->m_children.push_back(gumball_darwin);

	//Node* mc_laren = loader.load_from_resource("assets/models/McLaren.glb");
	//m_scene->m_children.push_back(mc_laren);

	//Node* shrek = loader.load_from_resource("assets/models/shrek/scene.gltf");
	//m_scene->m_children.push_back(shrek);

	//Node* cube1 = create_cube(0, 0, 0, {1.0f, 0.0f, 1.0f, 1.0f});
	//Node* cube2 = create_cube(0, 2, 0, {1.0f, 1.0f, 0.0f, 1.0f});
	//m_scene->m_children.push_back(cube1);
	//m_scene->m_children.push_back(cube2);

	m_renderer = Renderer::create();

	//SDL_SetRelativeMouseMode(SDL_TRUE); todo

	m_free_camera.m_aspect_ratio = float(m_width) / m_height;
}

void MyGame::on_event(SDL_Event& event)
{
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
    {
        if (SDL_GetRelativeMouseMode()) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
        } else {
            m_should_close = true;
        }
    }

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
    m_renderer->render(m_scene, m_free_camera);
}

RGE_define_game(MyGame)
