
#include <vulkan/vulkan.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "gpu_allocator.hpp"

using namespace rge;

// https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/quick_start.html

// ------------------------------------------------------------------------------------------------ GpuBuffer

bool GpuBuffer::is_allocated()
{
	return m_buffer || m_allocation;
}

// ------------------------------------------------------------------------------------------------ GpuAllocator

void GpuAllocator::destroy_buffer_if_any(GpuBuffer& handle)
{
	if (handle.m_buffer != VK_NULL_HANDLE || handle.m_allocation != VK_NULL_HANDLE) {
		vmaDestroyBuffer(this->m_allocator, handle.m_buffer, handle.m_allocation);
	}

	handle.m_buffer = VK_NULL_HANDLE;
	handle.m_allocation = VK_NULL_HANDLE;
}

void GpuAllocator::alloc_host_visible_buffer(
	GpuBuffer& handle,
	VkBufferUsageFlags buffer_usage,
	size_t size,
	bool persistently_mapped
)
{
	destroy_buffer_if_any(handle);

	VkBufferCreateInfo buf_info{};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.usage = buffer_usage;
	buf_info.size = size;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo alloc_info;
	alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	alloc_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	alloc_info.flags = persistently_mapped ? VMA_ALLOCATION_CREATE_MAPPED_BIT : NULL;

	vmaCreateBuffer(m_allocator, &buf_info, &alloc_info, &handle.m_buffer, &handle.m_allocation, nullptr);
}

void GpuAllocator::update_host_visible_buffer(
	GpuBuffer& handle,
	void* data,
	size_t size,
	size_t dst_offset
)
{
	void* mapped_data;

	if (!handle.m_allocation->IsPersistentMap()) {
		vmaMapMemory(m_allocator, handle.m_allocation, &mapped_data);
	} else {
		mapped_data = handle.m_allocation->GetMappedData();
	}

	std::memcpy(static_cast<uint8_t*>(mapped_data) + dst_offset, data, size);

	if (!handle.m_allocation->IsPersistentMap()) {
		vmaUnmapMemory(m_allocator, handle.m_allocation);
	}

	VkMemoryPropertyFlags mem_flags;
	vmaGetMemoryTypeProperties(m_allocator, handle.m_allocation->GetMemoryTypeIndex(), &mem_flags);
	if ((mem_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
		vmaFlushAllocation(m_allocator, handle.m_allocation, dst_offset, size);
	}
}

void GpuAllocator::alloc_device_only_buffer(
	GpuBuffer& handle,
	VkBufferUsageFlags buffer_usage,
	size_t size
)
{
	destroy_buffer_if_any(handle);

	VkBufferCreateInfo buf_info{};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.usage = buffer_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buf_info.size = size;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo alloc_info;
	alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	vmaCreateBuffer(m_allocator, &buf_info, &alloc_info, &handle.m_buffer, &handle.m_allocation, nullptr);
}

void GpuAllocator::update_device_only_buffer(
	GpuBuffer& handle,
	void* data,
	size_t size,
	size_t dst_offset
)
{
	GpuBuffer staging_buf;
	alloc_host_visible_buffer(staging_buf, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, false);
	update_host_visible_buffer(staging_buf, data, size, 0);

	VkCommandBufferAllocateInfo cmd_buf_alloc_info{};
	cmd_buf_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_buf_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	//cmd_buf_alloc_info.commandPool = m_cmd_pool; todo
	cmd_buf_alloc_info.commandBufferCount = 1;

	VkCommandBuffer cmd_buf;
	vkAllocateCommandBuffers(m_allocator->m_hDevice, nullptr, &cmd_buf);

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmd_buf, &begin_info);

	VkBufferCopy copy_region{};
	copy_region.srcOffset = 0;
	copy_region.dstOffset = dst_offset;
	copy_region.size = size;
	vkCmdCopyBuffer(cmd_buf, staging_buf.m_buffer, handle.m_buffer, 1, &copy_region);

	vkEndCommandBuffer(cmd_buf);

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd_buf;

	// todo
	vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphics_queue);

	vkFreeCommandBuffers(m_allocator->m_hDevice, m_cmd_pool, 1, &cmd_buf); // todo
	destroy_buffer_if_any(staging_buf);
}

void GpuAllocator::update_buffer(
	GpuBuffer& handle,
	void* data,
	size_t size,
	size_t dst_offset
)
{
	VkMemoryPropertyFlags memory_flags;
	vmaGetMemoryTypeProperties(m_allocator, handle.m_allocation->GetMemoryTypeIndex(), &memory_flags);
	if (memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		update_host_visible_buffer(handle, data, size, dst_offset);
	} else {
		update_device_only_buffer(handle, data, size, dst_offset);
	}
}
