#include "scene_graph.h"

#include <functional>

#include "../render_system.h"
#include "renderer\dx12\dx12_texture.h"
#include "renderer/dx12//dx12_texture_array.h"

namespace rlr
{

	Node::Node(SceneGraph& graph, RenderSystem& render_system, std::string const& name)
		: graph(graph),
		render_system(render_system),
		name(name),
		type_id(0),
		parent(nullptr),
		initialized(false)
	{

	}

	Node::~Node()
	{

	}

	void Node::Update()
	{

	}

	void Node::AddChild(std::shared_ptr<Node> node)
	{
		node->parent = shared_from_this();
		children.push_back(node);
	}

	std::shared_ptr<Node> Node::GetParent() const
	{
		return parent;
	}

	std::vector<std::shared_ptr<Node>> Node::GetChildren() const
	{
		return children;
	}

	std::string Node::GetName() const {
		return name;
	}

	RootNode::RootNode(SceneGraph& graph, RenderSystem& render_system, std::string const& name, int width, int height)
		: Node(graph, render_system, name)
	{
		Create(this->viewport, width, height);
	}

	RootNode::~RootNode()
	{
		Destroy(this->viewport);
	}

	void RootNode::Init()
	{

	}

	void RootNode::Render(CommandList* cmd_list, Camera const& camera, bool shadows)
	{

	}

	SceneGraph::SceneGraph(RenderSystem& render_system, int width, int height)
		: m_render_system(render_system),
		m_diffuse_matrix_transforms(false),
		root(CreateNode<RootNode>("Root", width, height))
	{
	}

	SceneGraph::~SceneGraph()
	{
	}

	void SceneGraph::InitAll()
	{
		using recursive_func_t = std::function<void(std::shared_ptr<Node>)>;

		root->Init();

		recursive_func_t recursive_init = [&recursive_init](std::shared_ptr<Node> node)
		{
			for (auto child : node->children)
			{
				child->Init();
				recursive_init(child);
			}
		};

		for (auto child : root->children)
		{
			child->Init();
			recursive_init(child);
		}
	}

	void SceneGraph::Update()
	{
		using recursive_func_t = std::function<void(std::shared_ptr<Node>)>;

		recursive_func_t recursive_update = [&recursive_update](std::shared_ptr<Node> node)
		{
			for (auto child : node->children)
			{
				child->Update();
				recursive_update(child);
			}
		};

		for (auto child : root->children)
		{
			child->Update();
			recursive_update(child);
		}
	}

	Viewport SceneGraph::GetViewport() const
	{
		return std::static_pointer_cast<RootNode>(root)->viewport;
	}

	void SceneGraph::ResizeViewport(int width, int height)
	{
		Create(std::static_pointer_cast<RootNode>(root)->viewport, width, height);
	}

} /* rlr */