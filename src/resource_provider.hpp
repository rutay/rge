#pragma once

#include <vector>

namespace rge::resource_provider
{
int get_size(char const* path);

int read(char const* path, uint8_t* buffer);
int read(char const* path, std::vector<uint8_t>& buffer);
}
