#include "camera.hpp"

#include <bgfx/bgfx.h>
#include <cstdio>

using namespace rge;

Camera::Camera()
	:
	m_position({0, 0, 0}),
	m_yaw(0),
	m_pitch(0),
	m_fov(45.0f),
	m_aspect_ratio(1.0f / 2.0f),
	m_near_plane(0.1f),
	m_far_plane(1000.0f)
{}

Camera::~Camera()
{}

bx::Vec3 Camera::forward() const
{
	return {
		bx::cos(m_pitch) * bx::sin(m_yaw),
		bx::sin(m_pitch),
		bx::cos(m_pitch) * bx::cos(m_yaw),
	};
}

bx::Vec3 Camera::right() const
{
	return {
		bx::sin(m_yaw - bx::kPiHalf),
		0,
		bx::cos(m_yaw - bx::kPiHalf),
	};
}

bx::Vec3 Camera::up() const
{
	return bx::cross(right(), forward());
}

void Camera::offset_position(bx::Vec3 const &axis, float intensity)
{
	offset_position(bx::mul(axis, intensity));
}

void Camera::offset_position(bx::Vec3 const &offset)
{
	m_position = bx::add(m_position, offset);
}

void Camera::offset_orientation(float yaw_offset, float pitch_offset)
{
	m_yaw += yaw_offset;
	m_pitch += pitch_offset;
}

void Camera::view(float dest[16]) const
{
	bx::mtxLookAt(dest, m_position, bx::add(m_position, forward()), up());
}

void Camera::projection(float dest[16]) const
{
	bx::mtxProj(dest, m_fov, m_aspect_ratio, m_near_plane, m_far_plane, bgfx::getCaps()->homogeneousDepth);
}

void Camera::matrix(float dest[16]) const
{
	float view_mat[16];
	view(view_mat);

	float proj_mat[16];
	projection(proj_mat);

	bx::mtxMul(dest, proj_mat, view_mat);
}

// ------------------------------------------------------------------------------------------------
// Controller
// ------------------------------------------------------------------------------------------------

void Camera_Movement_KeyboardController::update(Camera &camera, float delta) const
{
	auto intensity = m_speed * delta;
	if (m_forward.m_pressed)  camera.offset_position(camera.forward(), intensity);
	if (m_backward.m_pressed) camera.offset_position(bx::mul(camera.forward(), -1), intensity);
	if (m_right.m_pressed)    camera.offset_position(camera.right(), intensity);
	if (m_left.m_pressed)     camera.offset_position(bx::mul(camera.right(), -1), intensity);
	if (m_up.m_pressed)       camera.offset_position(camera.up(), intensity);
	if (m_down.m_pressed)     camera.offset_position(bx::mul(camera.up(), -1), intensity);

}

void Camera_Movement_KeyboardController::on_sdl_event(SDL_Event &event)
{
	if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
		auto key = event.key.keysym.sym;
		auto pressed = event.type == SDL_KEYDOWN;
		if (m_forward.m_key == key)  m_forward.m_pressed = pressed;
		if (m_backward.m_key == key) m_backward.m_pressed = pressed;
		if (m_left.m_key == key)     m_left.m_pressed = pressed;
		if (m_right.m_key == key)    m_right.m_pressed = pressed;
		if (m_up.m_key == key)       m_up.m_pressed = pressed;
		if (m_down.m_key == key)     m_down.m_pressed = pressed;
	}
}

void Camera_Orientation_MouseController::on_sdl_event(Camera& camera, SDL_Event &event) const
{
	if (event.type == SDL_MOUSEMOTION) {
		camera.offset_orientation(
			event.motion.xrel * m_yaw_speed,
			(-event.motion.yrel) * m_pitch_speed
		);
	}
}




