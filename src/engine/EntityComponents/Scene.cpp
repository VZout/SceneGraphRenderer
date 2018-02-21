//#include <Engine/EntityComponents/Scene.h>
//using namespace loki;
//
//Scene::Scene(Camera *_c): wr(new WorldRenderer()), main(_c) { }
//Scene::~Scene() { delete wr; }
//WorldRenderer *Scene::getRenderer() { return wr; }
//void Scene::render(GL *gl, Camera *c) { wr->render(gl, c == nullptr ? main : c); }
//
//Camera *Scene::getMainCamera() { return main; }
//void Scene::setMainCamera(Camera *c) {
//	main = c;
//}
//
//bool loki::Scene::instantiate(GameObject * go)
//{
//	if (go != nullptr)
//	{
//		go->setScene(this);
//		add(go);
//		go->start();
//		return true;
//	}
//	return false;
//}
//
//GameObject *loki::Scene::instantiate() {
//	GameObject *go = new GameObject();
//	go->setScene(this);
//	add(go);
//	go->start();
//	return go;
//}
//
//void loki::Scene::update(f64 deltaTime)
//{
//	for (u32 i = 0; i < size(); ++i)
//	{
//		container[i]->update(deltaTime);
//	}
//
//	for (auto v : purgedObjects)
//	{
//		remove(v, false);
//		purgedObjects.remove(v, true);
//	}
//}
