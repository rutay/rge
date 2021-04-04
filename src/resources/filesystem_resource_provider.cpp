#include "resource_provider.hpp"

#include <fstream>

using namespace rge;

int ResourceProvider::get_size(std::filesystem::path path)
{
	return read<char>(path, nullptr);
}

template<typename T>
int ResourceProvider::read(std::filesystem::path path, T* buffer)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);

	if (!file.is_open()) {
		return -1;
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	if (buffer) {
		file.read(reinterpret_cast<char*>(buffer), size);
	}
	return size / sizeof(T);
}
