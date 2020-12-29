#ifndef __EMSCRIPTEN__

#include "resource_provider.hpp"

#include <fstream>
#include <iostream>

size_t ResourceProvider::__request_impl(char const* path, uint8_t* buffer)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        std::cerr << "Resource not found: " << path << std::endl;
        return 0;
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (buffer)
        file.read(reinterpret_cast<char*>(buffer), size);

    return size;
}

#endif
