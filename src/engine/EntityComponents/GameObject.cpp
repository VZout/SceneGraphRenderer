#include "GameObject.h"
#include "Scene.h"

using namespace rle;

GameObject::GameObject(bool is_static)
	: STATIC(is_static), Entity()
{

}

const bool &GameObject::IS_STATIC() 
{ 
	return STATIC; 
}

GameObject::~GameObject()
{
}

void GameObject::Destroy()
{
	if (!GetDestroyed())
	{
		marked_for_destroy = true;
		// get scene and purge object
	}
}

void GameObject::Start() {}

void GameObject::Update(double deltaTime)
{
	for (Component *c : components)
	{
		c->update(deltaTime);
	}

	for (Component *c : components)
	{
		c->post_update(deltaTime);
	}
}

std::vector<Component*> GameObject::GetComponents()
{
	return components;
}

// TODO: reimplement with RLRenderer
//Scene *GameObject::get_scene()
//{
//	return scene;
//}

bool GameObject::GetDestroyed()
{
	return marked_for_destroy;
}

// TODO: reimplement with RLRenderer
//void GameObject::set_scene(loki::Scene *scene)
//{
//	this->scene = scene;
//}
