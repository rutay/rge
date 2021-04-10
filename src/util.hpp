
#pragma once

#include <cmath>
#include <stdint.h>

namespace rge
{

// ------------------------------------------------------------------------------------------------
// Vec
// ------------------------------------------------------------------------------------------------

template<size_t Dim, typename T>
struct Vec;

// ------------------------------------------------------------------------------------------------
// Vec3
// ------------------------------------------------------------------------------------------------

template<typename T>
struct Vec<3, T>
{
    float x, y, z;
};

using Vec3 = Vec<3, float>;

// ------------------------------------------------------------------------------------------------
// Vec4
// ------------------------------------------------------------------------------------------------

template<typename T>
struct Vec<4, T>
{
	union {
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };
	};

	Vec<4, T>() = default;

	Vec<4, T>(T x, T y, T z, T w) :
	    x(x), y(y), z(z), w(w)
	{}

	Vec<4, T>(Vec<3, T> xyz, T w) :
	    x(xyz.x),
	    y(xyz.y),
	    z(xyz.z),
	    w(w)
	{}

	inline void operator=(Vec<3, T> const& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}
};

using Vec4 = Vec<4, float>;

    inline Vec4 rgba_f_from_hex(uint32_t hex)
    {
        return Vec4(
                (float) ((hex >> 24) & 0xff) / 255.0f,
                (float) ((hex >> 16) & 0xff) / 255.0f,
                (float) ((hex >> 8) & 0xff) / 255.0f,
                (float) (hex & 0xff) / 255.0f
                );
    }
}

// ------------------------------------------------------------------------------------------------
// Quaternion
// ------------------------------------------------------------------------------------------------

struct Quaternion
{
	float x, y, z, w;
};
