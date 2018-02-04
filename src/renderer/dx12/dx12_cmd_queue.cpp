#include "../interface.h"

#include "dx12_cmd_queue.h"

namespace rlr {

void Create(CommandQueue* cmd_queue, Device* device, CmdQueueType type) {
	D3D12_COMMAND_QUEUE_DESC cmd_queue_desc = {};
	cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	switch (type) {
	case CmdQueueType::CMD_QUEUE_DIRECT:
		cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		break;
	default:
		cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		throw "Unknown or unsuported commdn queue type";
		break;
	}

	HRESULT hr = device->native->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(&cmd_queue->native));
	if (FAILED(hr)) {
		throw "Failed to create DX12 command queue.";
	}
	cmd_queue->native->SetName(L"Thisismycommandqueue");
}

void Execute(CommandQueue* cmd_queue, std::vector<CommandList> cmd_lists, Fence* fence) {
	ID3D12CommandList** lists = new ID3D12CommandList*[cmd_lists.size()];
	for (auto i = 0; i < cmd_lists.size(); i++) {
		lists[i] = cmd_lists[i].native;
	}

	cmd_queue->native->ExecuteCommandLists(cmd_lists.size(), lists);

	Signal(fence, cmd_queue);
}

void Destroy(CommandQueue* cmd_queue) {
	cmd_queue->native->Release();
}

} /* rlr */
