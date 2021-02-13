#pragma once

#include <bx/math.h>
#include <SDL.h>

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

    // ------------------------------------------------------------------------------------------------
    // Controller
	// ------------------------------------------------------------------------------------------------

	struct Camera_Movement_KeyboardController
	{
		struct Action
		{
			SDL_Keycode const m_key;
			bool m_pressed;
		};

		float m_speed = 1.0f;

		Action m_forward  = { SDLK_w, false };
		Action m_backward = { SDLK_s, false };
		Action m_left     = { SDLK_a, false };
		Action m_right    = { SDLK_d, false };
		Action m_up       = { SDLK_SPACE, false };
		Action m_down     = { SDLK_LSHIFT, false };

		void update(Camera& camera, float delta) const;

		void on_sdl_event(SDL_Event& event);
	};

	struct Camera_Orientation_MouseController
	{
		float m_yaw_speed   = bx::kPi / 10; // rad/pixel
		float m_pitch_speed = bx::kPi / 10; // rad/pixel

		void on_sdl_event(Camera& camera, SDL_Event& event) const;
	};
}
