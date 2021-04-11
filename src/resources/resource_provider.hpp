
#pragma once

#include <filesystem>
#include <vector>

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

	template<is_resource_type R, typename T>
	int read(R resource, T* buffer)
	{
		return read(ResourceManager::get_resource_path(resource), buffer);
	}

	template<is_resource_type R, typename T>
	int read(R resource, std::vector<T>& buffer)
	{
		auto path = ResourceManager::get_resource_path(resource);
		int size = get_size(path);
		if (size < 0) {
			return size;
		}
		buffer.resize(size);
		return read(path, buffer.data());
	}

	inline int read_to_string(Resource resource, std::string& buffer)
    {
        int size = get_size(resource); // todo req path once
        if (size < 0)
            return size;

	    buffer.resize(size + 1);
	    read(resource, buffer.data());

	    // Resize will zero all the bytes for the set size by itself.
	    //buffer[size] = '\0';

	    return size;
    }
};
}
