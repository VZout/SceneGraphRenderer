#include "vk_command_list.h"

#include "../interface.h"

namespace rlr {

void Allocate(CommandList& cmd_list, Device& device, unsigned int num) {
	SET_INTERNAL_DEVICE_PTR(cmd_list, device)

	vk::Device n_device = device.device;

	cmd_list.m_num = num;

	vk::CommandBufferAllocateInfo alloc_info = {};
	alloc_info.commandPool = device.cmd_pool;
	alloc_info.level = vk::CommandBufferLevel::ePrimary;
	alloc_info.commandBufferCount = num;

	cmd_list.m_native = (vk::CommandBuffer*)malloc(sizeof(CommandList) * num);
	vk::Result r = n_device.allocateCommandBuffers(&alloc_info, cmd_list.m_native);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to allocate command buffers.");
	}
}

void Destroy(CommandList& cmd_list) {
	vk::Device n_device = GET_NATIVE_INTERNAL_DEVICE(cmd_list);
	vk::CommandPool n_cmd_pool = GET_NATIVE_INTERNAL_CMD_POOL(cmd_list);

	n_device.freeCommandBuffers(n_cmd_pool, cmd_list.m_num, cmd_list.m_native);
}

void Begin(CommandList& cmd_list, RenderWindow& render_window, bool temp) {
	vk::CommandBufferBeginInfo begin_info = {};
	if (!temp)
		begin_info.flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue;
	else {
		begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	}

	std::vector<vk::ClearValue> clear_values = {};
	vk::RenderPassBeginInfo render_pass_info = {};

	if (!temp) {
		if (render_window.m_has_depth_attachment) {
			clear_values.resize(2);
			clear_values[1].depthStencil.setDepth(1.0f);
			clear_values[1].depthStencil.setStencil(0);
		}
		else {
			clear_values.resize(1);
		}
		clear_values[0].color = render_window.m_clear_color;

		render_pass_info.renderPass = render_window.m_render_pass;
		render_pass_info.framebuffer = render_window.m_framebuffers[render_window.m_frame_idx];
		render_pass_info.renderArea.offset = (0, 0);
		render_pass_info.renderArea.extent = vk::Extent2D(render_window.m_width, render_window.m_height); //FIXME: Hardcoded extend. should probably be the size of the swapchain or render pass.
		render_pass_info.clearValueCount = clear_values.size();
		render_pass_info.pClearValues = clear_values.data();
	}

	vk::CommandBuffer n_cmd_list = cmd_list.m_native[render_window.m_frame_idx];

	n_cmd_list.begin(begin_info);
	if (!temp) n_cmd_list.beginRenderPass(&render_pass_info, vk::SubpassContents::eInline);

	cmd_list.m_current_frame_idx = render_window.m_frame_idx;
}

void End(CommandList& cmd_list, RenderWindow& render_window, bool temp) {
	vk::CommandBuffer n_cmd_list = cmd_list.m_native[render_window.m_frame_idx];

	if (!temp) n_cmd_list.endRenderPass();
	n_cmd_list.end();
}

//
void Bind(CommandList& cmd_list, Viewport& viewport) {
	vk::CommandBuffer n_cmd_list = cmd_list.m_native[cmd_list.m_current_frame_idx];

	n_cmd_list.setViewport(0, 1, &viewport.m_viewport);
	n_cmd_list.setScissor(0, 1, &viewport.m_scissor);
}

void Bind(CommandList& cmd_list, PipelineState& pipeline_state) {
	vk::CommandBuffer n_cmd_list = cmd_list.m_native[cmd_list.m_current_frame_idx];

	n_cmd_list.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_state.m_native);
}

void BindVertexBuffer(CommandList& cmd_list, StagingBuffer& buffer) {
	vk::CommandBuffer n_cmd_list = cmd_list.m_native[cmd_list.m_current_frame_idx];

	vk::DeviceSize offsets[] = { 0 };
	n_cmd_list.bindVertexBuffers(0, 1, &buffer.buffer, offsets);
}

void BindIndexBuffer(CommandList& cmd_list, StagingBuffer& buffer, unsigned int offset) {
	vk::CommandBuffer n_cmd_list = cmd_list.m_native[cmd_list.m_current_frame_idx];

	n_cmd_list.bindIndexBuffer(buffer.buffer, offset, vk::IndexType::eUint16);
}

void Draw(CommandList& cmd_list, unsigned int vertex_count, unsigned int inst_count) {
	vk::CommandBuffer n_cmd_list = cmd_list.m_native[cmd_list.m_current_frame_idx];

	n_cmd_list.draw(vertex_count, inst_count, 0, 0);
}

void DrawIndexed(CommandList& cmd_list, unsigned int idx_count, unsigned int inst_count) {
	vk::CommandBuffer n_cmd_list = cmd_list.m_native[cmd_list.m_current_frame_idx];

	n_cmd_list.drawIndexed(idx_count, inst_count, 0, 0, 0);
}

} /* rlr */
