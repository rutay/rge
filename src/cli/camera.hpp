#pragma once

#include <bx/math.h>
#include <SDL.h>

#include <optional>

namespace rge
{
    struct Camera
    {
    	bx::Vec3 m_position;
        float m_rotation[3];
        float m_scale[3];

        float m_aspect_ratio;
        float m_fov;
        float m_near_plane;
        float m_far_plane;

        Camera();
        ~Camera();

        void orientation(float result[16]) const;

        bx::Vec3 forward() const;
        bx::Vec3 up() const;
        bx::Vec3 right() const;

        void view_matrix(float result[16]) const;
        void projection_matrix(float result[16]) const;
    };

    struct FreeCamera : public Camera
	{
		float m_min_pitch = -bx::kPiQuarter;
		float m_max_pitch = bx::kPiQuarter;

    	void move_forward(float intensity);
    	inline void move_backward(float intensity) { move_forward(-intensity); }

    	void move_right(float intensity);
		inline void move_left(float intensity) { move_right(-intensity); }

    	void move_up(float intensity);
		inline void move_down(float intensity) { move_up(-intensity); }

    	void rotate_yaw(float offset);
    	void rotate_pitch(float offset);
	};

    // ------------------------------------------------------------------------------------------------
    // Controller
	// ------------------------------------------------------------------------------------------------

	struct FreeCamera_KeyboardMovementController
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

		void update(FreeCamera& camera, float delta) const;

		void on_sdl_event(SDL_Event& event);
	};

	struct FreeCamera_OrientationMouseController
	{
		float m_yaw_speed   = bx::kPi / 1000; // rad/pixel
		float m_pitch_speed = bx::kPi / 1000; // rad/pixel

		void on_sdl_event(FreeCamera& camera, SDL_Event& event) const;
	};
}
