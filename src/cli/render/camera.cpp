#include "camera.hpp"

#include <bgfx/bgfx.h>

// ------------------------------------------------------------------------------------------------ Camera

Camera::Camera() :
    m_position({0, 0, 0}),
    m_yaw(0),
    m_pitch(0),
    m_fov(45.0f),
    m_aspect_ratio(1.0f / 2.0f),
    m_near_plane(0.1f),
    m_far_plane(1000.0f)
{}

Camera::~Camera() {}

bx::Vec3 Camera::forward()
{
    return {
        bx::cos(m_pitch) * bx::sin(m_yaw),
        bx::sin(m_pitch),
        bx::cos(m_pitch) * bx::cos(m_yaw),
    };
}

bx::Vec3 Camera::right()
{
    return {
        bx::sin(m_yaw - bx::kPiHalf),
        0,
        bx::cos(m_yaw - bx::kPiHalf),
    };
}

bx::Vec3 Camera::up()
{
    return bx::cross(right(), forward());
}

void Camera::offset_position(bx::Vec3 const& axis, float intensity)
{
    offset_position(bx::mul(axis, intensity));
}

void Camera::offset_position(bx::Vec3 const& offset)
{
    m_position = bx::add(m_position, offset);
}

void Camera::offset_orientation(float yaw_offset, float pitch_offset)
{
    m_yaw += yaw_offset;
    m_pitch += pitch_offset;
}

void Camera::view(float dest[16])
{
    bx::mtxLookAt(dest, m_position, bx::add(m_position, forward()), up());
}

void Camera::projection(float dest[16])
{
    bx::mtxProj(dest, m_fov, m_aspect_ratio, m_near_plane, m_far_plane, bgfx::getCaps()->homogeneousDepth);
}

void Camera::matrix(float dest[16])
{
    float view_mat[16];
    view(view_mat);
    
    float proj_mat[16];
    projection(proj_mat);

    bx::mtxMul(dest, proj_mat, view_mat);
}
