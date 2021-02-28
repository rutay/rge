
#include "resource_provider.hpp"

#include <fstream>

using namespace rge;

int resource_provider::get_size(char const* path)
{
	return read(path, nullptr);
}

int resource_provider::read(char const* path, uint8_t* buffer)
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
	return size;
}

int resource_provider::read(char const* path, std::vector<uint8_t>& buffer)
{
	buffer.resize(get_size(path));
	return read(path, buffer.data());
}
