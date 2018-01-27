#pragma once

#include "scene_graph.h"

namespace rlr {

class ViewportNode : public Node {
public:
	ViewportNode(SceneGraph& graph, RenderSystem& render_system, std::string const& name, Viewport viewport);
	ViewportNode(SceneGraph& graph, RenderSystem& render_system, std::string const& name, int width, int height);
	virtual ~ViewportNode();

	virtual void Init() override;
	virtual void Render(CommandList& cmd_list, Camera const& camera, bool shadows) override;

	Viewport m_viewport;
};

}