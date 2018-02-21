#include "Component.h"
#include "GameObject.h"
#include "Scene.h"

using namespace rle;

Component::Component()
{
}

Component::Component(GameObject * owner)
	: owner(owner)
{
}

Component::~Component()
{
}

void Component::start()
{ }

void Component::update(double deltaTime)
{ }

void Component::post_update(double deltaTime)
{ }

GameObject *Component::get_owner()
{
	return owner;
}

GameObject *Component::instantiate(GameObject * newObject)
{
	GameObject *go = newObject;
	
	if (newObject == nullptr)
	{
		go = new GameObject();
	}

	//if (owner->get_scene()->instantiate(go))
	//{
	//	return go;
	//}

	return nullptr;
}
