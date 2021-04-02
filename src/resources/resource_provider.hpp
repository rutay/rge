
#pragma once

#include <filesystem>

#include "resource.hpp"
#include "resource_manager.hpp"

namespace rge
{
class ResourceProvider
{
private:
	ResourceProvider() = default;

public:
	static int get_size(std::filesystem::path path);

	static int get_size(Resource resource)
	{
		return get_size(ResourceManager::get_resource_path(resource));
	}

	template<typename T>
	static int read(std::filesystem::path path, T* buffer);

	template<typename T>
	static int read(Resource resource, T* buffer)
	{
		return read(ResourceManager::get_resource_path(resource), buffer);
	}

	template<typename T>
	static int read(Resource resource, std::vector<T>& buffer)
	{
		auto path = ResourceManager::get_resource_path(resource);
		int size = get_size(path);
		buffer.resize(size);
		return read(path, buffer.data());
	}
};
}
