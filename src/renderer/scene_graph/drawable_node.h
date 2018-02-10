#pragma once

#include "scene_graph.h"

#include "renderer\math\vec.hpp"
#include "renderer\dx12\dx12_texture.h"

namespace rlr
{

class DrawableNode : public Node
{
public:
	DrawableNode(SceneGraph& graph, RenderSystem& render_system, std::string const& name, std::string const& pipeline_id, bool cast_shadows = true, bool instanced = false, int instanced_batch_id = 0);
	virtual ~DrawableNode();

	void SetTextures(std::vector<Texture*> textures);

	virtual void Init() override;
	virtual void Render(CommandList& cmd_list, Camera const& camera, bool shadows) override;

	void SetInstancedPos(fm::vec3 pos);

public:
	bool cast_shadows;
	bool instanced;

	unsigned int instanced_batch_id = 0;
	fm::vec3 instanced_pos;

	std::string material_id;
	std::string pipeline_id;
	Model* model;
	TextureArray* ta;
	ConstantBuffer* const_buffer;
	ConstantBuffer* shadow_const_buffer;
};

} /* rlr */