#include "camera.hpp"

#include <bgfx/bgfx.h>
#include <cstdio>

using namespace rge;

Camera::Camera() :
	m_position{0, 0, 0},
	m_rotation{0, 0, 0},
	m_scale{1, 1, 1},
	m_fov(90.0f),
	m_aspect_ratio(1.0f),
	m_near_plane(0.1f),
	m_far_plane(1000.0f)
{}

Camera::~Camera()
{}

void Camera::orientation(float result[16]) const
{
	float rot_x[16], rot_y[16];

	// The rotations are counterclockwise if the axis of rotation points towards the viewer.
	// That's ok for the pitch (m_rotation[1]), but the rotation verse must be inverted for the yaw.
	bx::mtxRotateX(rot_x, m_rotation[1]);
	bx::mtxRotateY(rot_y, -m_rotation[0]);

	bx::mtxMul(result, rot_x, rot_y);
}

bx::Vec3 Camera::forward() const
{
	float o[16];
	orientation(o);
	return bx::mul(bx::Vec3(0, 0, 1), o); // Without rotation the camera's aligned along with the Z axis.
}

bx::Vec3 Camera::right() const
{
	float o[16];
	orientation(o);
	return bx::mul(bx::Vec3(1, 0, 0), o);
}

bx::Vec3 Camera::up() const
{
	float o[16];
	orientation(o);
	return bx::mul(bx::Vec3(0, 1, 0), o);
}

void Camera::view_matrix(float result[16]) const
{
	// V = S(1/s) * inverse(R) * T(-t)

	float r[16], inv_r[16];
	orientation(r);
	bx::mtxInverse(inv_r, r);

	float inv_t[16];
	bx::mtxTranslate(inv_t, -m_position.x, -m_position.y, -m_position.z);

	bx::mtxMul(result, inv_t, inv_r);
}

void Camera::projection_matrix(float result[16]) const
{
	bx::mtxProj(result, m_fov, m_aspect_ratio, m_near_plane, m_far_plane, bgfx::getCaps()->homogeneousDepth);
}

// ------------------------------------------------------------------------------------------------
// FreeCamera
// ------------------------------------------------------------------------------------------------

void FreeCamera::move_forward(float intensity)
{
	bx::Vec3 direction = forward();
	bx::Vec3 increment = bx::mul(direction, intensity);
	m_position = bx::add(m_position, increment);
}

void FreeCamera::move_right(float intensity)
{
	m_position = bx::add(m_position, bx::mul(right(), intensity));
}

void FreeCamera::move_up(float intensity)
{
	m_position = bx::add(m_position, bx::mul(up(), intensity));
}

void FreeCamera::rotate_yaw(float offset)
{
	m_rotation[0] += offset;
	//printf("yaw = %f\n", m_rotation[0]);
}

void FreeCamera::rotate_pitch(float offset)
{
	m_rotation[1] += offset;
	m_rotation[1] = bx::clamp(m_rotation[1], m_min_pitch, m_max_pitch);
	//printf("pitch = %f\n", m_rotation[1]);
}

// ------------------------------------------------------------------------------------------------
// Controller
// ------------------------------------------------------------------------------------------------

void FreeCamera_KeyboardMovementController::update(FreeCamera &camera, float delta) const
{
	auto intensity = m_speed * delta;
	if (m_forward.m_pressed)  camera.move_forward(intensity);
	if (m_backward.m_pressed) camera.move_backward(intensity);
	if (m_right.m_pressed)    camera.move_right(intensity);
	if (m_left.m_pressed)     camera.move_left(intensity);
	if (m_up.m_pressed)       camera.move_up(intensity);
	if (m_down.m_pressed)     camera.move_down(intensity);

}

void FreeCamera_KeyboardMovementController::on_sdl_event(SDL_Event &event)
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

void FreeCamera_OrientationMouseController::on_sdl_event(FreeCamera& camera, SDL_Event &event) const
{
	if (event.type == SDL_MOUSEMOTION) {
		camera.rotate_yaw(event.motion.xrel * m_yaw_speed);
		camera.rotate_pitch(-event.motion.yrel * m_pitch_speed);


		printf("yaw=%f pitch=%f\n", camera.m_rotation[0], camera.m_rotation[1]);
	}
}




