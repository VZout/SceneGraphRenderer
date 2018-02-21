#pragma once
#include "Entity.h"
#include "ComponentMetadata.h"

// TODO: adhere style guide
// Time of completion: before friday, 23 feb
namespace rle
{
	class GameObject;

	class Component : public Entity
	{
	public:
		Component();
		Component(GameObject *owner);
		~Component();

		ComponentMetadata *metadata;

		virtual void start();
		virtual void update(double deltaTime);
		virtual void post_update(double deltaTime);

		GameObject *get_owner();

		virtual GameObject *instantiate(GameObject *newObject = nullptr);

	private:
		GameObject *owner;
	};
}