cmake_minimum_required(VERSION 3.9.0)

##### INCLUDE DIRECTORIES #####
include_directories(../src)

#if (USE_D3D12)
add_definitions(-DUSE_D3D12)
#elseif (USE_VULKAN)
#add_definitions(-DUSE_VULKAN -DENABLE_VK_VALIDATION_LAYERS)
#endif()

##### RENDER SYSTEM TEST #####
project(RendererTest)

file(GLOB_RECURSE SOURCES "renderer/*.cpp")
file(GLOB_RECURSE HEADERS "renderer/*.h")

add_executable(RenderTest ${SOURCES} ${HEADERS})
set_target_properties(RenderTest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/../")
target_link_libraries(RenderTest RLRenderer glfw3)
set_target_properties(RenderTest PROPERTIES FOLDER "Tests")
