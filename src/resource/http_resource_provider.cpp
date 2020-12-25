#ifdef __EMSCRIPTEN__

#include "resource_provider.hpp"

size_t ResourceProvider::request(char const* resource_name, uint8_t* buffer)
{
    // TODO: Fancy HTTP request in order to retrieve the specified resource.

    return 0;
}

#endif
