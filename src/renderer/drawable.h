#pragma once

#include <string>
#include "render_system.h"
#include "model.h"
#include "interface.h"

namespace rlr {

class Drawable {
private:
	RenderSystem& render_system;

public:
	Drawable(RenderSystem& render_system);
	~Drawable();

	bool cast_shadows = true;

	bool instanced = false;
	fm::vec3 instanced_pos;

	void SetTextures(std::vector<Texture*> textures);

	std::string material_id;
	std::string pipeline_id;
	Model model;
	TextureArray ta;
	ConstantBuffer const_buffer;
	ConstantBuffer shadow_const_buffer;
	
	int instanced_rendering_group_id = 0;
};

} /* rlr */