#include "vk_fence.h"

#include "../interface.h"
#include "vk_internal.h"

namespace rlr {

void Create(Fence& fence, Device& device) {
	SET_INTERNAL_DEVICE_PTR(fence, device);

	vk::FenceCreateInfo fence_info;
	fence.native = device.device.createFence(fence_info);
}

void Destroy(Fence& fence) {
	GET_NATIVE_INTERNAL_DEVICE(fence).destroyFence(fence.native);
}

void WaitFor(Fence& fence, Device& device) {
	vk::Device n_device = device.device;

	vk::Result r = n_device.waitForFences(1, &fence.native, true, std::numeric_limits<uint64_t>::max());
	if (r == vk::Result::eSuccess) {
		n_device.resetFences(1, &fence.native);
	}
}

} /* rlr */
