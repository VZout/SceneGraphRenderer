#include "drawable_node.h"

#include "renderer\render_system.h"

namespace rlr
{

SceneNode::SceneNode(SceneGraph& graph, RenderSystem& render_system, std::string const& name, bool movable)
	: Node(graph, render_system, name), movable(movable), transform(new Transform())
{
}

SceneNode::~SceneNode()
{
	delete transform;
}

void SceneNode::Init()
{
	if (!movable && transform->RequiresUpdate())
	{
		for (auto child : children) {
			auto child_sn = std::static_pointer_cast<SceneNode>(child);
			child_sn->GetTransform()->SetParentChanged();
		}

		if (auto p = std::dynamic_pointer_cast<SceneNode>(parent))
		{
			transform->Update(p->GetTransform());
		}
		else
		{
			transform->Update(nullptr);
		}
	}
	else if (!movable && transform->ParentChanged())
	{
		auto p = std::static_pointer_cast<SceneNode>(parent);
		transform->UpdateWorldOnly(p->GetTransform());
	}
}

void SceneNode::Update()
{
	if (movable && transform->RequiresUpdate())
	{
		for (auto child : children) {
			auto child_sn = std::static_pointer_cast<SceneNode>(child);
			child_sn->GetTransform()->SetParentChanged();
		}

		if (auto p = std::dynamic_pointer_cast<SceneNode>(parent))
		{
			transform->Update(p->GetTransform());
		}
		else
		{
			transform->Update(nullptr);
		}
	}
	else if (movable && transform->ParentChanged())
	{
		auto p = std::static_pointer_cast<SceneNode>(parent);
		transform->UpdateWorldOnly(p->GetTransform());
	}
}

void SceneNode::Render(CommandList* cmd_list, Camera const& camera, bool shadows)
{
	// nothing.
}

Transform* SceneNode::GetTransform() const
{
	return transform;
}

bool SceneNode::IsMovable() const
{
	return movable;
}

} /* rlr */
