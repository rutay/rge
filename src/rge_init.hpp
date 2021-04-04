#pragma once

#include "resources_def.hpp"
#include "cli/renderer/material_serializers_def.hpp"

namespace rge
{
	void init()
	{
		resources::init();
		material_serializers::init();
	}
}

