cmake_minimum_required(VERSION 3.9.0)

##### INCLUDE DIRECTORIES #####
include_directories(../src)

##### RENDERER MODULE #####
project(RLRenderer)

if (USE_D3D12)
	add_definitions(-DUSE_D3D12)
elseif (USE_VULKAN)
	add_definitions(-DUSE_VULKAN -DENABLE_VK_VALIDATION_LAYERS)
endif()

file(GLOB SOURCES "renderer/*.cpp")
file(GLOB HEADERS "renderer/*.h")
file(GLOB_RECURSE IMGUI_SOURCES "renderer/imgui/*.cpp")
file(GLOB_RECURSE IMGUI_INL "renderer/imgui/*.inl")
file(GLOB_RECURSE IMGUI_HEADERS "renderer/imgui/*.h")
file(GLOB_RECURSE CG_SOURCES "renderer/scene_graph/*.cpp")
file(GLOB_RECURSE CG_HEADERS "renderer/scene_graph/*.h")
file(GLOB_RECURSE MATH_SOURCES "/imgui/*.cpp")
file(GLOB_RECURSE MATH_HEADERS "math/imgui/*.h")

if (USE_D3D12)
	message(STATUS "RoguelikeEngine - Using DX12")
	file(GLOB_RECURSE API_SOURCES "renderer/dx12/*.cpp")
	file(GLOB_RECURSE API_HEADERS "renderer/dx12/*.h")
elseif (USE_VULKAN)
	message(STATUS "Falcon Renderer - Using Vulkan")
	file(GLOB_RECURSE API_SOURCES "renderer/vk/*.cpp")
	file(GLOB_RECURSE API_HEADERS "renderer/vk/*.h")
endif()

add_library(RLRenderer ${SOURCES} ${HEADERS} ${API_HEADERS} ${API_SOURCES} ${IMGUI_SOURCES} ${IMGUI_INL} ${IMGUI_HEADERS} ${CG_SOURCES} ${CG_HEADERS} ${MATH_SOURCES} ${MATH_HEADERS})

source_group("Generic"      FILES ${SOURCES} ${HEADERS})
source_group("API Impl"     FILES ${API_HEADERS} ${API_SOURCES})
source_group("ImGui"        FILES ${IMGUI_SOURCES} ${IMGUI_HEADERS})

if (USE_D3D12)
	target_link_libraries(RLRenderer dxguid.lib d3d12.lib dxgi.lib d3dcompiler.lib glfw assimp)
elseif (USE_VULKAN)
	target_link_libraries(RLRenderer vulkan-1 dxguid.lib d3d12.lib dxgi.lib d3dcompiler.lib glfw assimp)
endif()

add_dependencies(RLRenderer glfw assimp)

##### UTILITY MODULE #####
project(RLUtility)

file(GLOB_RECURSE SOURCES "util/*.cpp")
file(GLOB_RECURSE HEADERS "util/*.h")

add_library(RLUtility ${SOURCES} ${HEADERS})
target_link_libraries(RLUtility RLRenderer d3d12.lib dxgi.lib d3dcompiler.lib)
