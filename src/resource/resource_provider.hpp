#pragma once

#include <vector>

namespace ResourceProvider
{
    size_t request(char const* path, uint8_t* buffer);
    size_t request(char const* path, std::vector<uint8_t>& buffer);
};
