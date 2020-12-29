#include "resource_provider.hpp"

size_t ResourceProvider::request(char const* path, uint8_t* buffer, bool log)
{
    if (log) printf("Requesting resource: %s\n", path);

    return __request_impl(path, buffer);
}

size_t ResourceProvider::request(char const* path, std::vector<uint8_t>& buffer, bool log)
{
    size_t size = request(path, NULL, false);
    if (log) printf("Resource size: %lld bytes\n", size);

    buffer.resize(size);

    return request(path, buffer.data(), log);
}
