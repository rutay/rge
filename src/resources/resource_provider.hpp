
#pragma once

#include <filesystem>

#include "resource.hpp"
#include "resource_manager.hpp"

namespace rge
{
namespace ResourceProvider
{
	int get_size(std::filesystem::path path);

	inline int get_size(Resource resource)
	{
		return get_size(ResourceManager::get_resource_path(resource));
	}

	int read(std::filesystem::path path, uint8_t* buffer);

	template<typename T>
	int read(std::filesystem::path path, T* buffer)
	{
		int size = read(path, reinterpret_cast<uint8_t*>(buffer));
		return size > 0 ? size / sizeof(T) : size;
	}

	template<typename T>
	int read(Resource resource, T* buffer)
	{
		return read(ResourceManager::get_resource_path(resource), buffer);
	}

	template<typename T>
	int read(Resource resource, std::vector<T>& buffer)
	{
		auto path = ResourceManager::get_resource_path(resource);
		int size = get_size(path);
		buffer.resize(size);
		return read(path, buffer.data());
	}
};
}
