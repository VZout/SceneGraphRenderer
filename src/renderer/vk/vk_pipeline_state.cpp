#include "vk_pipeline_state.h"

#include <stdexcept>
#include <iostream>

#include "vk_device.h"
#include "vk_root_signature.h"
#include "../interface.h"
#include "../vertex.h" // FIXME: Should'nt be nessessary.

namespace rlr {

void SetVertexShader(PipelineState& pipeline_state, Shader& shader) {
	pipeline_state.m_v_shader_stage_info.stage = vk::ShaderStageFlagBits::eVertex;
	pipeline_state.m_v_shader_stage_info.module = shader.module;
	pipeline_state.m_v_shader_stage_info.pName = "main";
}

void SetFragmentShader(PipelineState& pipeline_state, Shader& shader) {
	pipeline_state.m_f_shader_stage_info.stage = vk::ShaderStageFlagBits::eFragment;
	pipeline_state.m_f_shader_stage_info.module = shader.module;
	pipeline_state.m_f_shader_stage_info.pName = "main";
}

void SetRootSignature(PipelineState& pipeline_state, RootSignature* root_signature) {
	pipeline_state.m_root_signature = root_signature;

	//FIXME: is this the correct location? Cross reference dx12.
	pipeline_state.m_binding_desc = Vertex::GetBindingDescription();
	pipeline_state.m_attrib_descs = Vertex::GetAttributeDescriptions();

	// FIXME: Move to finalize?
	pipeline_state.m_vertex_input_info.vertexBindingDescriptionCount = 1;
	pipeline_state.m_vertex_input_info.pVertexBindingDescriptions = &pipeline_state.m_binding_desc; // Optional
	pipeline_state.m_vertex_input_info.vertexAttributeDescriptionCount = pipeline_state.m_attrib_descs.size();
	pipeline_state.m_vertex_input_info.pVertexAttributeDescriptions = pipeline_state.m_attrib_descs.data(); // Optional
}

void Finalize(PipelineState& pipeline_state, Device& device, RenderWindow& render_window) {
	vk::Device n_device = device.device;

	vk::PipelineShaderStageCreateInfo shader_stages[] = {
		pipeline_state.m_v_shader_stage_info,
		pipeline_state.m_f_shader_stage_info
	};

	vk::PipelineInputAssemblyStateCreateInfo input_assambler_info = {};
	input_assambler_info.topology = vk::PrimitiveTopology::eTriangleList;
	input_assambler_info.primitiveRestartEnable = VK_FALSE;

	vk::Extent2D swap_chain_extent(500, 500); //FIXME: Hardcoded shit. Dont want the fucking viewport here anyway. This is only causing inconsistencies with DX12 without any benefit.

	vk::Viewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = 500;
	viewport.height = 500;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor = {};
	scissor.offset = vk::Offset2D{ 0, 0 };
	scissor.extent = swap_chain_extent;

	vk::PipelineViewportStateCreateInfo viewport_state_info = {};
	viewport_state_info.viewportCount = 1;
	viewport_state_info.pViewports = &viewport;
	viewport_state_info.scissorCount = 1;
	viewport_state_info.pScissors = &scissor;

	vk::PipelineRasterizationStateCreateInfo rasterizer_info = {};
	rasterizer_info.depthClampEnable = VK_FALSE;
	rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
	rasterizer_info.polygonMode = vk::PolygonMode::eFill;
	rasterizer_info.lineWidth = 1.0f;
	rasterizer_info.cullMode = vk::CullModeFlagBits::eNone;
	rasterizer_info.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizer_info.depthBiasEnable = VK_FALSE;
	rasterizer_info.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer_info.depthBiasClamp = 0.0f; // Optional
	rasterizer_info.depthBiasSlopeFactor = 0.0f; // Optional

	vk::PipelineDepthStencilStateCreateInfo depth_stencil_info = {};
	depth_stencil_info.depthTestEnable = VK_TRUE;
	depth_stencil_info.depthWriteEnable = VK_TRUE;
	depth_stencil_info.depthCompareOp = vk::CompareOp::eLess;
	depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_info.minDepthBounds = 0.0f; // Optional
	depth_stencil_info.maxDepthBounds = 1.0f; // Optional
	depth_stencil_info.stencilTestEnable = VK_FALSE;
	//depth_stencil_info.front = {}; // Optional
	//depth_stencil_info.back = {}; // Optional

	vk::PipelineMultisampleStateCreateInfo multisampling_info = {};
	multisampling_info.sampleShadingEnable = VK_FALSE;
	multisampling_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
	multisampling_info.minSampleShading = 1.0f; // Optional
	multisampling_info.pSampleMask = nullptr; /// Optional
	multisampling_info.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling_info.alphaToOneEnable = VK_FALSE; // Optional

	vk::PipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
	color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	color_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
	color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eOne;
	color_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;

	vk::PipelineColorBlendStateCreateInfo color_blending_info = {};
	color_blending_info.logicOpEnable = VK_FALSE;
	color_blending_info.logicOp = vk::LogicOp::eCopy; // Optional
	color_blending_info.attachmentCount = 1;
	color_blending_info.pAttachments = &color_blend_attachment;
	color_blending_info.blendConstants[0] = 0.0f; // Optional
	color_blending_info.blendConstants[1] = 0.0f; // Optional
	color_blending_info.blendConstants[2] = 0.0f; // Optional
	color_blending_info.blendConstants[3] = 0.0f; // Optional

	std::vector<vk::DynamicState> dynamic_states = {
		//vk::DynamicState::eViewport,
		//vk::DynamicState::eScissor,
	};

	vk::PipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.dynamicStateCount = dynamic_states.size();
	dynamic_state.pDynamicStates = dynamic_states.data();

	vk::DescriptorSetLayout set_layouts[] = { pipeline_state.m_root_signature->m_desc_set_layout, pipeline_state.m_root_signature->m_desc_set_layout_img };

	vk::PipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.setLayoutCount = 2; // Optional
	pipeline_layout_info.pSetLayouts = set_layouts; // Optional
	pipeline_layout_info.pushConstantRangeCount = 0; // Optional
	pipeline_layout_info.pPushConstantRanges = 0; // Optional

	vk::Result r = n_device.createPipelineLayout(&pipeline_layout_info, nullptr, &pipeline_state.m_layout);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to create pipeline layout.");
	}

	vk::GraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &pipeline_state.m_vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assambler_info;
	pipeline_info.pViewportState = &viewport_state_info;
	pipeline_info.pRasterizationState = &rasterizer_info;
	pipeline_info.pMultisampleState = &multisampling_info;
	pipeline_info.pDepthStencilState = &depth_stencil_info;
	pipeline_info.pColorBlendState = &color_blending_info;
	pipeline_info.pDynamicState = nullptr; // Optional
	pipeline_info.layout = pipeline_state.m_layout;
	pipeline_info.renderPass = render_window.m_render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = nullptr; // Optional
	pipeline_info.basePipelineIndex = -1; // Optional

	r = n_device.createGraphicsPipelines(nullptr, 1, &pipeline_info, nullptr, &pipeline_state.m_native);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to create graphics pipeline.");
	}
}

void Destroy(PipelineState& pipeline_state, Device& device) {
	vk::Device n_device = device.device;
	n_device.destroyPipelineLayout(pipeline_state.m_layout);
	n_device.destroyPipeline(pipeline_state.m_native);
}

} /* rlr */
