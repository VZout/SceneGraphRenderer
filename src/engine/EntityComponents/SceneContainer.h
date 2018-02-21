#pragma once

#include <vector>
#include <Types/Types.h>
#include "Scene.h"
#include <Interfaces/Container.h>
#include <Render/Camera.h>

namespace loki {

	class SceneContainer : public Container<Scene> {

	public:

		SceneContainer() { cameras.add(new Camera(Vec3())); }
		Container<Camera> *getCameras() { return &cameras; }

		void update(f64 delta) {
			for (u32 i = 0; i < size(); ++i)
				container[i]->update(delta);
		}

	private:

		Container<Camera> cameras;

	};

}