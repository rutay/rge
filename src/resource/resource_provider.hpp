#pragma once

#include <vector>

namespace ResourceProvider
{
    size_t __request_impl(char const* path, uint8_t* buffer);

    size_t request(char const* path, uint8_t* buffer, bool log = true);
    size_t request(char const* path, std::vector<uint8_t>& buffer, bool log = true);
};
