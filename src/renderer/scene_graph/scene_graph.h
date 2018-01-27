#pragma once

#include <vector>
#include <memory>
#include <array>
#include <string>

namespace rlr {
	class RenderSystem;
	class CommandList;
	class Camera;
	class Model;
	class ConstantBuffer;
	class TextureArray;
}

class SceneGraph;

#include "../dx12/dx12_viewport.h"

class Node : public std::enable_shared_from_this<Node> {
	friend class SceneGraph;
public:
	Node(SceneGraph& graph, rlr::RenderSystem& render_system, std::string const& name);
	virtual ~Node();

	virtual void Init() = 0;
	virtual void Render(rlr::CommandList& cmd_list, rlr::Camera const& camera, bool shadows) = 0;

	virtual void OnAdded() {}

	void AddChild(std::shared_ptr<Node> node);
	std::shared_ptr<Node> GetParent() const;
	std::vector<std::shared_ptr<Node>> GetChildren() const;
	std::string GetName() const;

protected:
	SceneGraph & m_graph;

	std::string m_name;
	unsigned int m_type_id;

	rlr::RenderSystem& m_render_system;

	std::shared_ptr<Node> m_parent;
	std::vector<std::shared_ptr<Node>> m_children;

	bool m_initialized;
};

class RootNode : public Node {
public:
	RootNode(SceneGraph& graph, rlr::RenderSystem& render_system, std::string const& name) : Node(graph, render_system, name) {

	}
	virtual ~RootNode() {

	}

	virtual void Init() final {

	}

	virtual void Render(rlr::CommandList& cmd_list, rlr::Camera const& camera, bool shadows) final {

	}
};

class SceneGraph {
public:
	SceneGraph(rlr::RenderSystem& render_system);
	~SceneGraph();
	
	std::shared_ptr<Node> root;

	void InitAll();
	void Optimize();

	template<typename T, class ... Types>
	[[nodiscard]] std::shared_ptr<T> CreateNode(std::string const& name, Types ... args) {
		return std::make_shared<T>(*this, m_render_system, name, args...);
	}

	template<typename T, class ... Types>
	std::shared_ptr<T> CreateChildNode(std::shared_ptr<Node> parent, std::string const& name, Types ... args) {
		auto node = std::make_shared<T>(*this, m_render_system, name, args...);
		parent->AddChild(node);
		return node;
	}

	//std::vector<std::shared_ptr<DrawableNode>> m_shadow_pass_objects;
	rlr::Viewport m_current_viewport;

private:
	rlr::RenderSystem& m_render_system;

	bool m_diffuse_matrix_transforms;
};

/*
template<typename T, class ... Types>
std::shared_ptr<Node> [[nodiscard]] CreateNode(Types ... args) {
	return std::make_shared<T>(args...);
}
*/