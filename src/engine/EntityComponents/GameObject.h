#pragma once
#include <vector>
#include <string>

#include "Entity.h"
#include "Component.h"

namespace rle
{
	class GameObject : public Entity
	{
	public:
		GameObject(bool isStatic = false);
		~GameObject();

		void Destroy();

		void Start();

		void Update(double deltaTime);

		template<typename T, typename ... Args>
		T *AddComponent(Args ... args);

		template<typename T, typename ... Args>
		T *AddNamedComponent(std::string name, Args ... args);

		template<class T = typename std::enable_if<std::is_base_of<Component, T>::value>::type>
		T *GetComponent();
		template<class T = typename std::enable_if<std::is_base_of<Component, T>::value>::type>
		T *GetComponentWithName(std::string name);

		template<typename T, typename ... Args>
		T *RequireComponent(Args ... args);

		std::vector<Component*> GetComponents();
		//Scene *get_scene();

		bool GetDestroyed();
		//void set_scene(Scene *scene);

		const bool &IS_STATIC();

		// TODO: move to getter
		std::vector<GameObject*> children;

	private:
		const bool STATIC;

		std::vector<Component*> components;
		//Scene *scene;

		template<class T = typename std::enable_if<std::is_base_of<Component, T>::value>::type>
		T *ConstructComponent(T *t, std::string name);

		bool marked_for_destroy = false;
	};

	template<typename T, typename ... Args>
	T *GameObject::AddComponent(Args ... args)
	{
		return AddNamedComponent<T>("", args...);
	}

	template<typename T, typename ... Args>
	T *GameObject::AddNamedComponent(std::string name, Args ... args)
	{
		T *ret = new T(args...);
		ret->set_name(name);
		ret->start();

		components.push_back(ret);

		return ret;
	}

	template<class T>
	T *GameObject::GetComponent()
	{
		for (Component *component : components)
		{
			T *c = dynamic_cast<T*>(component);
			if (c != nullptr)
			{
				return c;
			}
		}
		return nullptr;
	}

	template<class T>
	T *GameObject::GetComponentWithName(std::string name)
	{
		for (Component *component : components)
		{
			T *c = dynamic_cast<T*>(component);
			if (c != nullptr)
			{
				if (c->get_name() == name)
				{
					return c;
				}
			}
		}
		return nullptr;
	}

	template<class T>
	T *GameObject::ConstructComponent(T *t, std::string name)
	{
		if (dynamic_cast<Component*>(t))
		{
			t->set_name(name);
			components.add(t);
			return t;
		}
		return nullptr;
	}

	template<class T, typename ... Args>
	T *GameObject::RequireComponent(Args ... args)
	{
		T *t = GetComponent<T>();
		if (t == nullptr)
		{
			return AddComponent<T>(args...);
		}
		return t;
	}
}