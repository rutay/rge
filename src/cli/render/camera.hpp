#pragma once

#include <bx/math.h>

struct Camera
{
    bx::Vec3 m_position;
    float m_yaw;
    float m_pitch;

    float m_aspect_ratio;
    float m_fov;
    float m_near_plane;
    float m_far_plane;

    Camera();
    ~Camera();

    bx::Vec3 forward();
    bx::Vec3 right();
    bx::Vec3 up();

    void offset_position(bx::Vec3 const& axis, float intensity);
    void offset_position(bx::Vec3 const& offset);

    void offset_orientation(float yaw_offset, float pitch_offset);

    void view(float dest[16]);
    void projection(float dest[16]);
    void matrix(float dest[16]);
};
