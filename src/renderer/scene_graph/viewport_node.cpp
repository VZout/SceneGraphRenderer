#include "viewport_node.h"

#include "renderer\render_system.h"

namespace rlr {

ViewportNode::ViewportNode(SceneGraph& graph, rlr::RenderSystem& render_system, std::string const& name, Viewport viewport)
	: m_viewport(viewport),
	Node(graph, render_system, name) {
}

ViewportNode::ViewportNode(SceneGraph& graph, rlr::RenderSystem& render_system, std::string const& name, int width, int height)
	: Node(graph, render_system, name) {
	Create(m_viewport, width, height);
}

ViewportNode::~ViewportNode() {
}

void ViewportNode::Render(rlr::CommandList& cmd_list, rlr::Camera const& camera, bool shadows) {
	m_graph.m_current_viewport = m_viewport;
}

void ViewportNode::Init() {

}

}