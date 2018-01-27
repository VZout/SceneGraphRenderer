#pragma once

#include <memory>
#include <chrono>
#include <string>
#include <vector>
#include <iostream>
#include <d3d12.h>

#include "dx12/d3dx12.h"

namespace profiler {

struct Node {
	using Precision = long double;
	using Duration = std::chrono::duration<Precision>;

	using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

	std::string name;
	Precision start;
	Precision end;
	std::shared_ptr<Node> parent;
	std::vector<std::shared_ptr<Node>> childs;

	// gpu
	unsigned int heap_index = 0;
};

extern const int max_num_timers;
extern double gpu_tick_delta;
extern ID3D12Resource* query_result;
extern ID3D12QueryHeap* query_heap;
extern int next_index;

using Precision = long double;
using Duration = std::chrono::duration<Precision>;

extern std::vector<std::shared_ptr<Node>> root_nodes;
extern std::vector<std::shared_ptr<Node>> prev_root_nodes;

extern std::vector<std::shared_ptr<Node>>  root_gpu_nodes;
extern std::vector<std::shared_ptr<Node>>  prev_root_gpu_nodes;

extern std::shared_ptr<Node> current_node;
extern std::shared_ptr<Node> current_gpu_node;

static Precision Now() {
	auto time_point = std::chrono::high_resolution_clock::now();
	auto epoch = time_point.time_since_epoch();
	return std::chrono::duration_cast<Duration>(epoch).count();
}

static void NewFrame() {
	prev_root_nodes.clear();
	prev_root_gpu_nodes.clear();

	for (std::shared_ptr<Node> node : root_nodes) {
		prev_root_nodes.push_back(std::make_shared<Node>(*node));
	}

	for (std::shared_ptr<Node> node : root_gpu_nodes) {
		prev_root_gpu_nodes.push_back(std::make_shared<Node>(*node));
	}

	root_nodes.clear();
	root_gpu_nodes.clear();
}

static void InitProfiler(ID3D12Device* device, ID3D12CommandQueue* cmd_queue) {
	uint64_t gpu_freq;
	cmd_queue->GetTimestampFrequency(&gpu_freq);
	gpu_tick_delta = 1.0 / static_cast<double>(gpu_freq);

	D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
	queryHeapDesc.Count = max_num_timers * 2;
	queryHeapDesc.NodeMask = 1;
	queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&query_heap));
	query_heap->SetName(L"GPU Profiler Timestamp Query");

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_READBACK;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC BufferDesc;
	BufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	BufferDesc.Alignment = 0;
	BufferDesc.Width = sizeof(uint64_t) * max_num_timers * 2;
	BufferDesc.Height = 1;
	BufferDesc.DepthOrArraySize = 1;
	BufferDesc.MipLevels = 1;
	BufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	BufferDesc.SampleDesc.Count = 1;
	BufferDesc.SampleDesc.Quality = 0;
	BufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	BufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	device->CreateCommittedResource(
		&HeapProps,
		D3D12_HEAP_FLAG_NONE,
		&BufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&query_result));

	query_result->SetName(L"GPU Profiler Query Result");
}

static void SetChildTimestamps(std::shared_ptr<Node> node, uint64_t* data) {
	for (auto child : node->childs) {
		child->start = (Precision)data[child->heap_index] * gpu_tick_delta;
		child->end = (Precision)data[child->heap_index + 1] * gpu_tick_delta;
		SetChildTimestamps(child, data);
	}
}

static void ResolveRead(std::shared_ptr<Node> node, ID3D12GraphicsCommandList* cmd_list) {
	cmd_list->ResolveQueryData(query_heap, D3D12_QUERY_TYPE_TIMESTAMP, 0, max_num_timers * 2, query_result, 0);
	next_index = 0;

	uint64_t* data = nullptr;

	// Begin Readback
	D3D12_RANGE Range;
	Range.Begin = 0;
	Range.End = (max_num_timers * 2) * sizeof(uint64_t);
	query_result->Map(0, &Range, reinterpret_cast<void**>(&data));

	// Parse Data
	node->start = (Precision)data[node->heap_index] * gpu_tick_delta;
	node->end = (Precision)data[node->heap_index + 1] * gpu_tick_delta;
	SetChildTimestamps(node, data);

	// End Readback
	D3D12_RANGE EmptyRange = {};
	query_result->Unmap(0, &EmptyRange);
	data = nullptr;
}

static void GPUStartRoot(std::string name, ID3D12GraphicsCommandList* cmd_list) {
	std::shared_ptr<Node> node = std::make_unique<Node>();
	node->name = name;
	node->parent = nullptr;
	node->heap_index = next_index;

	root_gpu_nodes.push_back(node);
	current_gpu_node = node;

	cmd_list->EndQuery(query_heap, D3D12_QUERY_TYPE_TIMESTAMP, next_index);
	next_index += 2;
}

static void GPUStopRoot(ID3D12GraphicsCommandList* cmd_list) {
	cmd_list->EndQuery(query_heap, D3D12_QUERY_TYPE_TIMESTAMP, current_gpu_node->heap_index + 1);

	ResolveRead(current_gpu_node, cmd_list);
}

static void GPUStartBlock(std::string name, ID3D12GraphicsCommandList* cmd_list) {
	std::shared_ptr<Node> node = std::make_unique<Node>();;
	node->name = name;
	node->parent = current_gpu_node;
	node->heap_index = next_index;

	current_gpu_node->childs.push_back(node);
	current_gpu_node = node;

	cmd_list->EndQuery(query_heap, D3D12_QUERY_TYPE_TIMESTAMP, next_index);
	next_index += 2;
}

static void GPUStopBlock(ID3D12GraphicsCommandList* cmd_list) {
	cmd_list->EndQuery(query_heap, D3D12_QUERY_TYPE_TIMESTAMP, current_gpu_node->heap_index + 1);

	current_gpu_node = current_gpu_node->parent;
}

static void StartRoot(std::string name) {
	std::shared_ptr<Node> node = std::make_unique<Node>();
	node->name = name;
	node->start = Now();
	node->parent = nullptr;

	root_nodes.push_back(node);
	current_node = node;
}

static void StopRoot() {
	current_node->end = Now();

	prev_root_nodes.clear();
}


static void StartBlock(std::string name) {
	std::shared_ptr<Node> node = std::make_unique<Node>();;
	node->name = name;
	node->start = Now();
	node->parent = current_node;

	current_node->childs.push_back(node);
	current_node = node;
}

static void StopBlock() {
	current_node->end = Now();
	current_node = current_node->parent;
}

static void PrintNode(std::shared_ptr<Node> node) {
	for (auto child : node->childs) {
		//for (unsigned int i = 0; i < child->depth; i++)
			//std::cout << "\t";

		auto diff = child->end - child->start;
		std::cout << child->name << ": " << diff << '\n';
		PrintNode(child);
	}
}

static void Print() {
	for (auto root_node : root_nodes) {
		std::cout << "Root node: " << root_node->name << "\n";
		PrintNode(root_node);
	}
}

} /* profiler */

//#define ENABLE_PROFILER
//#define ENABLE_PROFILER_LOCATION

#ifdef ENABLE_PROFILER_LOCATION
#define PARSE_NAME(n) std::string("[") + std::string(__FILE__) + std::string(":") + std::to_string(__LINE__) + std::string("] ") + std::string(n)
#else
#define PARSE_NAME(n) n
#endif

#ifdef ENABLE_PROFILER
#define PROFILER_BEGIN_ROOT_GPU(name, cmd_list) profiler::GPUStartRoot(PARSE_NAME(name), cmd_list);
#define PROFILER_END_ROOT_GPU(cmd_list) profiler::GPUStopRoot(cmd_list);
#define PROFILER_BEGIN_GPU(name, cmd_list) profiler::GPUStartBlock(PARSE_NAME(name), cmd_list);
#define PROFILER_END_GPU(cmd_list) profiler::GPUStopBlock(cmd_list);

#define PROFILER_BEGIN_ROOT(name) profiler::StartRoot(PARSE_NAME(name));
#define PROFILER_END_ROOT() profiler::StopRoot();
#define PROFILER_BEGIN(name) profiler::StartBlock(PARSE_NAME(name));
#define PROFILER_END() profiler::StopBlock();
#else
#define PROFILER_BEGIN_ROOT_GPU(name, cmd_list)
#define PROFILER_END_ROOT_GPU(cmd_list)
#define PROFILER_BEGIN_GPU(name, cmd_list)
#define PROFILER_END_GPU(cmd_list)

#define PROFILER_BEGIN_ROOT(name)
#define PROFILER_END_ROOT()
#define PROFILER_BEGIN(name)
#define PROFILER_END()
#endif