#pragma once

#include "../interface.h"

namespace rlr {

FWRD_DEVICE_PTR

struct Resource {
	INTERNAL_DEVICE_PTR

	vk::Image image;
	vk::DeviceMemory memory;
	vk::ImageView view; //TODO: Not every image needs a view
};

} /* rlr */