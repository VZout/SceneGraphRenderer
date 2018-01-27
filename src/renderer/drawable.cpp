#ifdef USE_D3D12
#include "drawable.h"

namespace rlr {

Drawable::Drawable(RenderSystem& render_system) : render_system(render_system) {
	Create(const_buffer, render_system.device, sizeof(CBStruct));
	Create(shadow_const_buffer, render_system.device, sizeof(CBStruct));
}

Drawable::~Drawable() {
	Destroy(ta);
	Destroy(const_buffer);
	Destroy(shadow_const_buffer);
	Destroy(model);
}

void Drawable::SetTextures(std::vector<Texture*> textures) {
	Create(ta, render_system.device, textures);
}

} /* rlr */

#endif