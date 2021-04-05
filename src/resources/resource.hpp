
#pragma once

#include <cstdint>
#include <type_traits>

namespace rge
{
	using Resource = uint32_t;

	template<typename R>
	concept is_resource_type = requires(R resource)
	{
		std::is_enum<R>::value; // todo
	};

	namespace resources
	{
		enum class Shader;
		enum class MaterialShader;
		enum class Material;
		enum class Model;
		enum class Texture;
	}
}
