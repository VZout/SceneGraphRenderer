#include "drawable_node.h"

#include "renderer\render_system.h"

namespace rlr {

DrawableNode::DrawableNode(SceneGraph& graph, rlr::RenderSystem& render_system, std::string const& name, std::string const& pipeline_id, bool cast_shadows, bool instanced, int instanced_batch_id) : Node(graph, render_system, name),
	pipeline_id(pipeline_id), m_cast_shadows(cast_shadows), m_instanced(instanced), m_instanced_batch_id(instanced_batch_id)
{
	ta = new TextureArray(); // FIXME: Shouldn't happen here but by some resource manager.
}

DrawableNode::~DrawableNode() {
	delete ta;
}

void DrawableNode::SetTextures(std::vector<Texture*> textures) {
	rlr::Create(*ta, *m_render_system.device, textures);
}

void DrawableNode::Init() {
	const_buffer = new rlr::ConstantBuffer();
	shadow_const_buffer = new rlr::ConstantBuffer();
	Create(*const_buffer, *m_render_system.device, sizeof(rlr::CBStruct));
	Create(*shadow_const_buffer, *m_render_system.device, sizeof(rlr::CBStruct));

	if (m_instanced) {
		m_render_system.static_inst_needs_staging = true;

		auto it = m_render_system.static_instanced_batches.find(m_instanced_batch_id);
		if (it != m_render_system.static_instanced_batches.end()) {
			it->second->inst_positions.push_back(m_instanced_pos);
			it->second->num_inst_model++;
		}
		else {
			Batch* b = new Batch();
			b->inst_drawable = this;
			b->inst_positions.push_back(m_instanced_pos);
			b->num_inst_model = 1;
			m_render_system.static_instanced_batches.insert(std::pair<int, Batch*>(m_instanced_batch_id, b));
		}
	}
}

void DrawableNode::Render(rlr::CommandList& cmd_list, rlr::Camera const& camera, bool shadows) {
	if (m_instanced) return;

	Bind(cmd_list, m_graph.GetViewport()); // TODO: This can be optimized when a render pass starts.

	if (shadows && !m_cast_shadows)
		return;

	if (shadows) {
		Bind(cmd_list, m_render_system.shadow_viewport);
		m_render_system.BindPipelineOptimized(cmd_list, pipeline_id + "_shadow");
		Bind(cmd_list, m_render_system.shadow_projection_view_const_buffer, 2, m_render_system.render_window.frame_idx);
		Bind(cmd_list, *shadow_const_buffer, 0, m_render_system.render_window.frame_idx);
	}
	else {
		m_render_system.BindPipelineOptimized(cmd_list, pipeline_id);
		Bind(cmd_list, m_render_system.projection_view_const_buffer, 2, m_render_system.render_window.frame_idx);
		Bind(cmd_list, *const_buffer, 0, m_render_system.render_window.frame_idx);
	}

	std::vector<ID3D12DescriptorHeap*> heaps = { ta->texture_heap };
	cmd_list.native->SetDescriptorHeaps(heaps.size(), heaps.data());

	for (size_t i = 0; i < model->meshes.size(); i++) {
		rlr::Bind(cmd_list, *ta, 1); // TODO: Allow multiple texture arrays per object.
		BindVertexBuffer(cmd_list, model->meshes[i].vb);
		BindIndexBuffer(cmd_list, model->meshes[i].ib);

		DrawIndexed(cmd_list, model->meshes[i].indices.size(), 1);
	}
}

void DrawableNode::SetInstancedPos(fm::vec3 pos) {
	m_instanced_pos = pos;
}

}