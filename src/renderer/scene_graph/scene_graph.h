#pragma once

#include <vector>
#include <memory>
#include <array>
#include <string>

#include "../dx12/dx12_viewport.h"
#include "../interface.h"

namespace rlr {
	class RenderSystem;
	class CommandList;
	class Camera;
	class Model;
	class ConstantBuffer;
	class TextureArray;
}

class SceneGraph;


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
	friend class SceneGraph;
public:
	RootNode(SceneGraph& graph, rlr::RenderSystem& render_system, std::string const& name, int width, int height) : Node(graph, render_system, name) {
		rlr::Create(this->viewport, width, height);
	}

	virtual ~RootNode() {
		rlr::Destroy(this->viewport);
	}

	virtual void Init() final {

	}

	virtual void Render(rlr::CommandList& cmd_list, rlr::Camera const& camera, bool shadows) final {

	}

private:
	rlr::Viewport viewport;
};

class SceneGraph {
public:
	SceneGraph(rlr::RenderSystem& render_system, int width, int height);
	~SceneGraph();
	
	std::shared_ptr<Node> root;

	void InitAll();

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

	rlr::Viewport GetViewport() const;

private:
	rlr::RenderSystem& m_render_system;

	bool m_diffuse_matrix_transforms;
};