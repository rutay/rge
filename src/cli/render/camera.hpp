#pragma once

#include <bx/math.h>

namespace rge
{
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

        bx::Vec3 forward() const;
        bx::Vec3 right() const;
        bx::Vec3 up() const;

        void offset_position(bx::Vec3 const& axis, float intensity);
        void offset_position(bx::Vec3 const& offset);

        void offset_orientation(float yaw_offset, float pitch_offset);

        void view(float dest[16]) const;
        void projection(float dest[16]) const;
        void matrix(float dest[16]) const;
    };

    struct FreeCameraController
    {
        float m_last_cursor_x, m_last_cursor_y;
        float m_camera_rotation_speed;
    };
}
