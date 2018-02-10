#pragma once

#include <vector>
#include <memory>
#include <array>
#include <string>

#include "../dx12/dx12_viewport.h"
#include "../interface.h"

namespace rlr
{
	class RenderSystem;
	class Camera;
	class Model;
	RLR_HANDLE CommandList;
	RLR_HANDLE ConstantBuffer;
	RLR_HANDLE TextureArray;

	class SceneGraph;

	class Node : public std::enable_shared_from_this<Node>
	{
		friend class SceneGraph;
	public:
		Node(SceneGraph& graph, RenderSystem& render_system, std::string const& name);
		virtual ~Node();

		virtual void Init() = 0;
		virtual void Render(CommandList& cmd_list, Camera const& camera, bool shadows) = 0;
		virtual void OnAdded() {}

		void AddChild(std::shared_ptr<Node> node);
		std::shared_ptr<Node> GetParent() const;
		std::vector<std::shared_ptr<Node>> GetChildren() const;
		std::string GetName() const;

	protected:
		SceneGraph & graph;

		std::string name;
		unsigned int type_id;

		RenderSystem& render_system;

		std::shared_ptr<Node> parent;
		std::vector<std::shared_ptr<Node>> children;

		bool initialized;
	};

	class RootNode : public Node
	{
		friend class SceneGraph;
	public:
		RootNode(SceneGraph& graph, RenderSystem& render_system, std::string const& name, int width, int height);
		virtual ~RootNode();

		virtual void Init() final;
		virtual void Render(CommandList& cmd_list, Camera const& camera, bool shadows) final;

	private:
		Viewport viewport;
	};

	class SceneGraph
	{
	public:
		SceneGraph(RenderSystem& render_system, int width, int height);
		~SceneGraph();

		std::shared_ptr<Node> root;

		void InitAll();

		template<typename T, class ... Types>
		[[nodiscard]] std::shared_ptr<T> CreateNode(std::string const& name, Types ... args)
		{
			return std::make_shared<T>(*this, m_render_system, name, args...);
		}

		template<typename T, class ... Types>
		std::shared_ptr<T> CreateChildNode(std::shared_ptr<Node> parent, std::string const& name, Types ... args)
		{
			auto node = std::make_shared<T>(*this, m_render_system, name, args...);
			parent->AddChild(node);
			return node;
		}

		Viewport GetViewport() const;
		void ResizeViewport(int width, int height);

	private:
		RenderSystem& m_render_system;

		bool m_diffuse_matrix_transforms;
	};

} /* rlr */