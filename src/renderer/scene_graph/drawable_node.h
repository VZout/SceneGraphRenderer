#pragma once

#include "scene_node.h"

#include "renderer\math\vec.hpp"
#include "renderer\dx12\dx12_texture.h"

namespace rlr
{

class DrawableNode : public SceneNode
{
public:
	DrawableNode(SceneGraph& graph, RenderSystem& render_system, std::string const& name, std::string const& pipeline_id, bool movable = false, bool cast_shadows = true, bool instanced = false, int instanced_batch_id = 0);
	virtual ~DrawableNode();

	void SetTextures(std::vector<Texture*> textures);

	virtual void Init() override;
	virtual void Update() override;
	virtual void Render(CommandList* cmd_list, Camera const& camera, bool shadows) override;

	void SetInstancedPos(fm::vec3 pos);
	fm::vec3 GetInstancedPos() const;

	void ShouldCastShadows(bool val);
	bool IsCastingShadows() const;

	void ShouldRenderInstanced(bool val);
	bool IsInstanced() const;

	void ShouldRequireUpdate(bool val);
	bool RequiresUpdate() const;

	void SetModel(Model* model);
	Model* GetModel() const;

	TextureArray* GetTextureArray() const;
	ConstantBuffer* GetConstantBuffer() const;
	std::string GetMaterialID() const;
	std::string GetPipelineID() const;


private:
	bool cast_shadows;
	bool requires_cb_update;
	bool instanced;

	unsigned int instanced_batch_id = 0;
	fm::vec3 instanced_pos;

	std::string material_id;
	std::string pipeline_id;
	Model* model;
	TextureArray* ta;
	ConstantBuffer* const_buffer;
};

} /* rlr */
