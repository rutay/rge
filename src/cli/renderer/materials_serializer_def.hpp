
#pragma once

namespace rge
{
template<typename TMaterial> requires is_material<TMaterial>
class __BaseMaterialSerializer : public MaterialSerializer
{
private:

public:
	virtual void serialize_specialized(TMaterial const* material, uint8_t* data, size_t& size) const
	{
		reinterpret_cast<TMaterial*>(data)[0] = *material;
		size = sizeof(TMaterial);
	}

	void serialize(Material const* material, uint8_t* data, size_t& size) const
	{
		size = 0;

		reinterpret_cast<uint32_t*>(data)[0] = material->get_type_id();

		data += sizeof(uint32_t);
		size += sizeof(uint32_t);

		serialize_specialized(static_cast<TMaterial const*>(material), data, size);
	}
};
}

