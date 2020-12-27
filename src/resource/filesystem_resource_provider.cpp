#if !defined(__EMSCRIPTEN__)

#include "resource_provider.hpp"

#include <fstream>
#include <iostream>

size_t ResourceProvider::request(char const* path, uint8_t* buffer)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        std::cerr << "Resource not found: " << path << std::endl;
        throw;
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (buffer)
        file.read(reinterpret_cast<char*>(buffer), size);

    return size;
}

size_t ResourceProvider::request(char const* path, std::vector<uint8_t>& buffer)
{
    return ResourceProvider::request(path, buffer.data());
}

#endif
