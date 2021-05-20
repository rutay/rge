
#pragma once

#include "scene_graph_linearizer.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace rge
{
	class GpuAllocator;

	struct GpuBuffer
	{
		VkBuffer m_buffer = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;

		bool is_allocated();
	};

	class GpuAllocator
	{
	private:
		/*
		 * todo should be in rge::Platform
		VkInstance m_instance;
		VkPhysicalDevice m_physical_device;
		VkDevice m_device;
		VmaAllocator m_allocator;
		*/
	public:
		void destroy_buffer_if_any(GpuBuffer& buffer);

		void alloc_host_visible_buffer(
			GpuBuffer& handle,
			VkBufferUsageFlags buffer_usage,
			size_t size,
			bool persistently_mapped = false
		);

		void alloc_device_only_buffer(
			GpuBuffer& handle,
			VkBufferUsageFlags buffer_usage,
			size_t size
		);

		void update_host_visible_buffer(
			GpuBuffer& handle,
			void* data,
			size_t size,
			size_t dst_offset
		);

		void update_device_only_buffer(
			GpuBuffer& handle,
			void* data,
			size_t size,
			size_t dst_offset
		);

		void update_buffer(
			GpuBuffer& handle,
			void* data,
			size_t size,
			size_t dst_offset
		);
	};
}
