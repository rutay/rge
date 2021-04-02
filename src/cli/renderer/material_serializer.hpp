
#pragma once

namespace rge
{
class MaterialSerializer
{
public:
	virtual void serialize(Material const* material, uint8_t* data, size_t& size) const = 0;
};
}
