
#include "renderer.hpp"

#include "vk_utils.hpp"
#include "vk_mem_alloc.h"

using namespace rge::renderer;

void SimpleDrawPass::create_descriptor_set_layout()
{
	VkDescriptorSetLayoutBinding camera_binding; // camera
	camera_binding.binding = 0;
	camera_binding.descriptorCount = 1;
	camera_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	camera_binding.pImmutableSamplers = nullptr;
	camera_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info;
	descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_set_layout_info.bindingCount = 1;
	descriptor_set_layout_info.pBindings = &camera_binding;

	if (vkCreateDescriptorSetLayout(m_device, &descriptor_set_layout_info, nullptr, &m_descriptor_set_layout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout.");
	}
}

void SimpleDrawPass::create_graphics_pipeline(uint32_t width, uint32_t height)
{
	VkShaderModule vert_shader_mod = create_shader_module(resources::Shader::VertexShader_SimpleDraw);
	VkShaderModule frag_shader_mod = create_shader_module(resources::Shader::FragmentShader_SimpleDraw);

	VkPipelineShaderStageCreateInfo vert_shader_stage_info;
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_mod;
	vert_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo frag_shader_stage_info;
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_mod;
	frag_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages_info[] = {
		vert_shader_stage_info,
		frag_shader_stage_info
	};

	VkVertexInputBindingDescription vtx_binding;
	vtx_binding.binding = 0;
	vtx_binding.stride = sizeof(float) * (3 + 3 + 4); // position + normal + color
	vtx_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputBindingDescription instance_data_binding;
	instance_data_binding.binding = 1;
	instance_data_binding.stride = sizeof(float) * 16; // transform
	instance_data_binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	auto binding_descriptions = DrawCall::get_binding_descriptions();
	auto attribute_descriptions = DrawCall::get_attributes_descriptions();

	VkPipelineVertexInputStateCreateInfo vtx_input_info;
	vtx_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vtx_input_info.vertexBindingDescriptionCount = binding_descriptions.size();
	vtx_input_info.pVertexBindingDescriptions = binding_descriptions.data();
	vtx_input_info.vertexAttributeDescriptionCount = attribute_descriptions.size();
	vtx_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
	input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport;
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent = {width, height};

	VkPipelineViewportStateCreateInfo viewport_state_info{};
	viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_info.viewportCount = 1;
	viewport_state_info.pViewports = &viewport;
	viewport_state_info.scissorCount = 1;
	viewport_state_info.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer_info;
	rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_info.depthClampEnable = VK_FALSE;
	rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
	rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer_info.lineWidth = 1.0f;
	rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer_info.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo  multisampling_info;
	multisampling_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling_info.sampleShadingEnable = VK_FALSE;
	multisampling_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState color_blend_attachment;
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blend_info;
	color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_info.logicOpEnable = VK_FALSE;
	color_blend_info.attachmentCount = 1;
	color_blend_info.pAttachments = &color_blend_attachment;

	VkPipelineLayoutCreateInfo pipeline_layout_info;
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &m_descriptor_set_layout;
	//pipeline_layout_info.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &m_pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline.");
	}

	vkDestroyShaderModule(m_device, vert_shader_mod, nullptr);
	vkDestroyShaderModule(m_device, frag_shader_mod, nullptr);
}

void SimpleDrawPass::destroy_swapchain()
{
	vkDestroyDescriptorSetLayout(m_device, m_descriptor_set_layout, nullptr);
	vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
	vkDestroyPipeline(m_device, m_pipeline, nullptr);
}

void SimpleDrawPass::recreate_swapchain(uint32_t width, uint32_t height)
{
	destroy_swapchain();

	create_descriptor_set_layout();
	create_graphics_pipeline(width, height);
}

SimpleDrawPass::SimpleDrawPass(VmaAllocator allocator, uint32_t width, uint32_t height) :
	m_allocator(allocator)
{
	recreate_swapchain(width, height);
}

SimpleDrawPass::~SimpleDrawPass()
{
	destroy_swapchain();
}

void SimpleDrawPass::init_camera_buffer(Frame& frame)
{
	VkBufferCreateInfo buffer_info;
	buffer_info.size = sizeof(float) * (16 + 16 + 16);
	buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	VmaAllocationCreateInfo alloc_info;
	//alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	vmaCreateBuffer(m_allocator, &buffer_info, &alloc_info, &frame.m_camera_ubo, &frame.m_camera_ubo_allocation, nullptr);
}

void SimpleDrawPass::init_descriptor_set(Frame& frame)
{
	VkDescriptorBufferInfo camera_buffer_info;
	camera_buffer_info.buffer = frame.m_camera_ubo;
	camera_buffer_info.offset = 0;
	camera_buffer_info.range = sizeof(float) * (16 + 16 + 16);

	VkWriteDescriptorSet descriptor_write;
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.dstSet = frame.m_descriptor_set;
	descriptor_write.dstBinding = 0;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_write.descriptorCount = 1;
	descriptor_write.pBufferInfo = &camera_buffer_info;

	vkUpdateDescriptorSets(m_device, 1, &descriptor_write, 0, nullptr);
}

void SimpleDrawPass::fill_dst_dependency(VkSubpassDependency& dep)
{
	dep.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
}

void SimpleDrawPass::set_camera(Frame const& frame, float view[16], float proj[16])
{
	void* data;

	vmaMapMemory(m_allocator, frame.m_camera_ubo_allocation, &data);

	std::memcpy(reinterpret_cast<float*>(data), view, sizeof(float) * 16);
	std::memcpy(reinterpret_cast<float*>(data) + 16, proj, sizeof(float) * 16);

	vmaUnmapMemory(m_allocator, frame.m_camera_ubo_allocation);
}

void SimpleDrawPass::record_commands(Frame const& frame, BakedSceneGraph const& input)
{
	vkCmdBindPipeline(frame.m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
	vkCmdBindDescriptorSets(frame.m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &frame.m_descriptor_set, 0, nullptr);

	for (auto [_, draw_call] : input.m_draw_calls)
	{
		auto vertex_buffers_binding_descriptions = draw_call.get_vertex_buffers_binding_descriptions();

		// Vertex buffers
		for (int vertex_buffer_idx = 0; vertex_buffer_idx < draw_call.m_vertex_buffers.size(); vertex_buffer_idx++)
		{
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(
				frame.m_command_buffer,
			  	vertex_buffers_binding_descriptions[vertex_buffer_idx].binding,
			  	1,
			  	&draw_call.m_vertex_buffers[vertex_buffer_idx].m_handle,
			  	&offset
			);
		}

		// Indices buffer
		vkCmdBindIndexBuffer(
			frame.m_command_buffer,
			draw_call.m_indices_buffer.m_buffer.m_handle,
			0,
			draw_call.m_indices_buffer.m_index_type
		);

		// Instance buffer
		for (InstanceBuffer& instance_buffer : draw_call.m_instances_buffers)
		{
			if (instance_buffer.is_allocated())
			{
				auto instance_buffer_binding_description = draw_call.get_instance_buffer_binding_description();

				VkDeviceSize offset = 0;
				vkCmdBindVertexBuffers(
					frame.m_command_buffer,
					instance_buffer_binding_description.binding,
					1,
					&instance_buffer.m_buffer.m_handle,
					&offset
				);

				vkCmdDrawIndexed(
					frame.m_command_buffer,
					draw_call.m_indices_buffer.m_indices_count,
					instance_buffer.m_instances_count,
					0,
					0,
					0
				);
			}
		}
	}
}
