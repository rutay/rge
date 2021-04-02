#pragma once

#include "resources/resource.hpp"

namespace rge
{
namespace resources
{
enum Material;
};

struct Material
{
	virtual resources::Material get_resource() const = 0;
};
}
