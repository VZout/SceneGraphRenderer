#pragma once

#include "../interface.h"

#include <vulkan\vulkan.hpp>
#include <memory>

namespace rlr {

	struct RootSignature;

	struct PipelineState {
		vk::Pipeline m_native;
		vk::PipelineLayout m_layout;

		vk::PipelineShaderStageCreateInfo m_v_shader_stage_info;
		vk::PipelineShaderStageCreateInfo m_f_shader_stage_info;

		vk::PipelineVertexInputStateCreateInfo m_vertex_input_info;

		RootSignature* m_root_signature; //FIXME: Ugly reference to root signature.

		vk::VertexInputBindingDescription m_binding_desc;
		std::array<vk::VertexInputAttributeDescription, 3> m_attrib_descs;
	};

} /* rlr */
