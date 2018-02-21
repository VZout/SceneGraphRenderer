#pragma once
#include <vector>

#include "GameObject.h"

//namespace	 {
//	class Scene : public std::vector<GameObject> {
//
//	public:
//		Scene(Camera *c);
//		~Scene();
//		void render(GL *gl, Camera *c);
//		WorldRenderer *getRenderer();
//
//		Camera *getMainCamera();
//		void setMainCamera(Camera *c);
//
//		bool instantiate(GameObject *go);
//		GameObject *instantiate();
//
//		void update(double deltaTime);
//
//		std::vector<GameObject*> purgedObjects;
//	private:		
//		WorldRenderer *wr;
//
//		Camera *main;
//
//	};
//
//}