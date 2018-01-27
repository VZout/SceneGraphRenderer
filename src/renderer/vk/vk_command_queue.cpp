#include "vk_command_queue.h"

#include "../interface.h"

namespace rlr {

void Create(CommandQueue& cmd_queue, Device& device, CmdQueueType type) {
	SET_INTERNAL_DEVICE_PTR(cmd_queue, device)

	vk::Device n_device = device.device;

	int queue_family = -1;
	switch (type) {
	case CmdQueueType::CMD_QUEUE_DIRECT:
		queue_family = device.queue_family_indices.present_family;
		break;
	case CmdQueueType::CMD_QUEUE_COMPUTE:
		throw std::runtime_error("UNSUPORTED COMMAND QUEUE TYPE");
		queue_family = device.queue_family_indices.present_family;
		break;
	case CmdQueueType::CMD_QUEUE_COPY:
		queue_family = device.queue_family_indices.graphics_family;
		break;
	default:
		throw std::runtime_error("Unknown cmd queue type");
		break;
	}

	// TODO: Increment index.
	n_device.getQueue(queue_family, 0, &cmd_queue.native);
}

void Execute(CommandQueue& cmd_queue, std::vector<CommandList> cmd_lists, Fence fence) {
	std::vector<vk::CommandBuffer> buffers;
	for (auto i = 0; i < cmd_lists.size(); i++) {
		buffers.push_back(cmd_lists[i].m_native[cmd_lists[i].m_current_frame_idx]);
	}

	vk::SubmitInfo submit_info = {};
	submit_info.commandBufferCount = cmd_lists.size();
	submit_info.pCommandBuffers = buffers.data();

	//if (info.wait_stages) { FIXME
	//	submit_info.pWaitDstStageMask = info.wait_stages;
	//}

	if (fence.native)
		cmd_queue.native.submit(1, &submit_info, fence.native);
	else
		cmd_queue.native.submit(1, &submit_info, nullptr);
}

} /* rlr */

