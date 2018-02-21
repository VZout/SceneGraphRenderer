#pragma once

#include "scene_graph.h"

#include "../transform.h"

namespace rlr
{

class SceneNode : public Node
{
public:
	SceneNode(SceneGraph& graph, RenderSystem& render_system, std::string const& name, bool movable = false);
	virtual ~SceneNode();

	virtual void Init() override;
	virtual void Update() override;
	virtual void Render(CommandList* cmd_list, Camera const& camera, bool shadows) override;

	Transform* GetTransform() const;
	bool IsMovable() const;

protected:
	bool movable;

	Transform* transform;
};

} /* rlr */
