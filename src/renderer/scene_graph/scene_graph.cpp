#include "scene_graph.h"

#include <functional>

#include "../render_system.h"
#include "renderer\dx12\dx12_texture.h"
#include "renderer/dx12//dx12_texture_array.h"

Node::Node(SceneGraph& graph, rlr::RenderSystem& render_system, std::string const& name)
	: m_graph(graph),
	m_render_system(render_system),
	m_name(name),
	m_type_id(0),
	m_parent(nullptr),
	m_initialized(false) {

}

Node::~Node() {

}

void Node::AddChild(std::shared_ptr<Node> node) {
	node->m_parent = shared_from_this();
	m_children.push_back(node);
}

std::shared_ptr<Node> Node::GetParent() const {
	return m_parent;
}

std::vector<std::shared_ptr<Node>> Node::GetChildren() const {
	return m_children;
}

std::string Node::GetName() const {
	return m_name;
}

SceneGraph::SceneGraph(rlr::RenderSystem& render_system, int width, int height) 
	: m_render_system(render_system),
	m_diffuse_matrix_transforms(false),
	root(CreateNode<RootNode>("Root", width, height)) {
}

SceneGraph::~SceneGraph() {
}

void SceneGraph::InitAll() {
	using recursive_func_t = std::function<void(std::shared_ptr<Node>)>;
	
	root->Init();

	recursive_func_t recursive_init = [&recursive_init](std::shared_ptr<Node> node) {
		for (auto child : node->m_children) {
			child->Init();
			recursive_init(child);
		}
	};

	for (auto child : root->m_children) {
		child->Init();
		recursive_init(child);
	}
}

rlr::Viewport SceneGraph::GetViewport() const {
	return std::static_pointer_cast<RootNode>(root)->viewport;
}