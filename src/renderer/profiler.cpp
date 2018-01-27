#include "profiler.h"

namespace profiler {

	const int  max_num_timers = 20;
	double  gpu_tick_delta = 0.0;
	ID3D12Resource* query_result;
	ID3D12QueryHeap*  query_heap;
	int  next_index = 0;

	std::vector<std::shared_ptr<Node>> root_nodes;
	std::vector<std::shared_ptr<Node>> prev_root_nodes;

	std::vector<std::shared_ptr<Node>>  root_gpu_nodes;
	std::vector<std::shared_ptr<Node>> prev_root_gpu_nodes;

	std::shared_ptr<Node> current_node;
	std::shared_ptr<Node> current_gpu_node;

}