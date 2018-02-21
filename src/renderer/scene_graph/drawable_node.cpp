#include "drawable_node.h"

#include "renderer\render_system.h"

namespace rlr
{

DrawableNode::DrawableNode(SceneGraph& graph, RenderSystem& render_system, std::string const& name, std::string const& pipeline_id, bool movable, bool cast_shadows, bool instanced, int instanced_batch_id)
	: SceneNode(graph, render_system, name, movable),
	pipeline_id(pipeline_id), cast_shadows(cast_shadows), instanced(instanced), instanced_batch_id(instanced_batch_id), requires_cb_update(true)
{
}

DrawableNode::~DrawableNode()
{
	delete ta;
}

void DrawableNode::SetTextures(std::vector<Texture*> textures)
{
	Create(&ta, render_system.device, textures);
}

void DrawableNode::Init()
{
	SceneNode::Init();

	Create(&const_buffer, render_system.device, sizeof(CBStruct));

	if (instanced)
	{
		render_system.static_inst_needs_staging = true;

		auto it = render_system.static_instanced_batches.find(instanced_batch_id);
		if (it != render_system.static_instanced_batches.end())
		{
			it->second->inst_positions.push_back(instanced_pos);
			it->second->num_inst_model++;
		}
		else
		{
			Batch* b = new Batch();
			b->inst_drawable = this;
			b->inst_positions.push_back(instanced_pos);
			b->num_inst_model = 1;
			render_system.static_instanced_batches.insert(std::pair<int, Batch*>(instanced_batch_id, b));
		}
	}

	if (!movable)
	{
		if (!requires_cb_update) return;

		CBStruct cb_data;

		DirectX::XMStoreFloat4x4(&cb_data.model, transform->GetWorldModel());

		cb_data.instanced = IsInstanced();

		for (auto i = 0; i < 3; i++)
		{
			rlr::Update(const_buffer, i, sizeof(cb_data), &cb_data);
		}
	}
}

void DrawableNode::Update()
{
	SceneNode::Update();

	if (!movable || !requires_cb_update) return;

	CBStruct cb_data;

	DirectX::XMStoreFloat4x4(&cb_data.model, transform->GetWorldModel());

	cb_data.instanced = IsInstanced();

	for (size_t i = 0; i < model->meshes[0].skeleton.bone_mats.size(); i++)
	{
		cb_data.weightmatrices[i] = aiMatrix4x4(model->meshes[0].skeleton.bone_mats[i]);
	}

	rlr::Update(const_buffer, render_system.render_window.frame_idx, sizeof(cb_data), &cb_data);
}

void DrawableNode::Render(CommandList* cmd_list, Camera const& camera, bool shadows)
{
	if (instanced) return;


	if (shadows && !cast_shadows)
		return;

	if (shadows)
	{
		Bind(*cmd_list, render_system.shadow_viewport);
		render_system.BindPipelineOptimized(cmd_list, pipeline_id + "_shadow");
		Bind(cmd_list, render_system.shadow_projection_view_const_buffer, 2, render_system.render_window.frame_idx);
	}
	else
	{
		Bind(*cmd_list, graph.GetViewport()); // TODO: This can be optimized when a render pass starts.
		render_system.BindPipelineOptimized(cmd_list, pipeline_id);
		Bind(cmd_list, render_system.projection_view_const_buffer, 2, render_system.render_window.frame_idx);
	}

	Bind(cmd_list, const_buffer, 0, render_system.render_window.frame_idx);

	std::vector<ID3D12DescriptorHeap*> heaps = { ta->texture_heap };
	cmd_list->native->SetDescriptorHeaps(heaps.size(), heaps.data());

	for (size_t i = 0; i < model->meshes.size(); i++)
	{
		Bind(*cmd_list, *ta, 1); // TODO: Allow multiple texture arrays per object.
		BindVertexBuffer(cmd_list, model->meshes[i].vb);
		BindIndexBuffer(cmd_list, model->meshes[i].ib);

		DrawIndexed(cmd_list, model->meshes[i].indices.size(), 1);
	}
}

void DrawableNode::SetInstancedPos(fm::vec3 pos)
{
	instanced_pos = pos;
}

fm::vec3 DrawableNode::GetInstancedPos() const
{
	return instanced_pos;
}

void DrawableNode::ShouldCastShadows(bool val)
{
	cast_shadows = val;
}

bool DrawableNode::IsCastingShadows() const
{
	return cast_shadows;
}

void DrawableNode::ShouldRenderInstanced(bool val)
{
	instanced = val;
}

bool DrawableNode::IsInstanced() const
{
	return instanced;
}

void DrawableNode::ShouldRequireUpdate(bool val)
{
	requires_cb_update = val;
}

bool DrawableNode::RequiresUpdate() const
{
	return requires_cb_update;
}

void DrawableNode::SetModel(Model* model)
{
	this->model = model;
}

Model* DrawableNode::GetModel() const
{
	return model;
}

TextureArray* DrawableNode::GetTextureArray() const
{
	return ta;
}

ConstantBuffer* DrawableNode::GetConstantBuffer() const
{
	return const_buffer;
}

std::string DrawableNode::GetMaterialID() const
{
	return material_id;
}

std::string DrawableNode::GetPipelineID() const
{
	return pipeline_id;
}

} /* rlr */
