#pragma once

#include "resources/resource.hpp"

namespace rge
{
	template<typename T>
	concept is_material = requires(T const* material) {
		{ material->get_resource() }; // todo could be better
	};

	struct Material
	{
		virtual resources::Material get_resource() const = 0;
		//virtual size_t get_byte_size() const = 0;
	};
}
