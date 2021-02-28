
#include "utils.hpp"

using namespace rge::scene;

// ------------------------------------------------------------------------------------------------
// Point
// ------------------------------------------------------------------------------------------------

Geometry utils::PointGeometry = []
{
	static float vertices[]{
		0, 0, 0
	};
	static Buffer buffer{
		vertices,
		sizeof(vertices)
	};
	static BufferView buffer_view{
		.m_buffer = &buffer,
		.m_byte_offset = 0,
		.m_byte_length = sizeof(vertices),
		.m_byte_stride = 0
	};
	static Accessor accessor{
		.m_buffer_view = &buffer_view,
		.m_component_type = ComponentType::FLOAT,
		.m_num_components = 3,
		.m_byte_offset = 0,
		.m_count = 1,
		.m_normalized = false
	};
	return Geometry{
		.m_mode = DrawMode::POINTS,
		.m_attributes = {&accessor},
		.m_indices = nullptr
	};
}();

// ------------------------------------------------------------------------------------------------
// Line
// ------------------------------------------------------------------------------------------------

Geometry utils::LineGeometry = []
{
	static float vertices[]{
		0, 0, 0,
		1, 1, 1,
	};
	static Buffer buffer{
		vertices,
		sizeof(vertices)
	};
	static BufferView buffer_view{
		.m_buffer = &buffer,
		.m_byte_offset = 0,
		.m_byte_length = sizeof(vertices),
		.m_byte_stride = 0
	};
	static Accessor accessor{
		.m_buffer_view = &buffer_view,
		.m_component_type = ComponentType::FLOAT,
		.m_num_components = 3,
		.m_byte_offset = 0,
		.m_count = 2,
		.m_normalized = false
	};
	return Geometry{
		.m_mode = DrawMode::LINES,
		.m_attributes = {&accessor},
		.m_indices = nullptr
	};
}();

// ------------------------------------------------------------------------------------------------
// Cube
// ------------------------------------------------------------------------------------------------

float CubeGeometry_vertices[]{
	// front
	-1.0, -1.0,  1.0,
	1.0, -1.0,  1.0,
	1.0,  1.0,  1.0,
	-1.0,  1.0,  1.0,
	// back
	-1.0, -1.0, -1.0,
	1.0, -1.0, -1.0,
	1.0,  1.0, -1.0,
	-1.0,  1.0, -1.0
};

unsigned short CubeGeometry_indices[]{
	// front
	0, 1, 2,
	2, 3, 0,
	// right
	1, 5, 6,
	6, 2, 1,
	// back
	7, 6, 5,
	5, 4, 7,
	// left
	4, 0, 3,
	3, 7, 4,
	// bottom
	4, 5, 1,
	1, 0, 4,
	// top
	3, 2, 6,
	6, 7, 3
};

Geometry utils::CubeGeometry{
	.m_mode = DrawMode::TRIANGLES,
	.m_attributes = {
		new Accessor{
			.m_buffer_view = new BufferView{
				.m_buffer = new Buffer{
					.m_data = CubeGeometry_vertices,
					.m_byte_length = sizeof(CubeGeometry_vertices)
				},
				.m_byte_offset = 0,
				.m_byte_length = sizeof(CubeGeometry_vertices),
				.m_byte_stride = 0
			},
			.m_component_type = ComponentType::FLOAT,
			.m_num_components = 3,
			.m_byte_offset = 0,
			.m_count = sizeof(CubeGeometry_vertices) / (3 * sizeof(float)),
			.m_normalized = false
		}
	},
	.m_indices = new Accessor{
		.m_buffer_view = new BufferView{
			.m_buffer = new Buffer{
				.m_data = CubeGeometry_indices,
				.m_byte_length = sizeof(CubeGeometry_indices)
			},
			.m_byte_offset = 0,
			.m_byte_length = sizeof(CubeGeometry_indices),
			.m_byte_stride = 0
		},
		.m_component_type = ComponentType::UNSIGNED_SHORT,
		.m_num_components = 1,
		.m_byte_offset = 0,
		.m_count = sizeof(CubeGeometry_indices) / sizeof(unsigned short),
		.m_normalized = false
	}
};
