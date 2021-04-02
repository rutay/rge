#pragma once

#include "scene/scene.hpp"
#include "cli/camera.hpp"
#include "rge.hpp"

namespace rge
{
class Renderer
{
protected:
	Renderer() = default;

public:
	virtual void render(Node const* root_node, Camera const& camera) = 0;

	static Renderer* create();

	static void init();
};
}

