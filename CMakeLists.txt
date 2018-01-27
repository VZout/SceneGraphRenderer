project(RoguelikeEngine)

cmake_minimum_required(VERSION 3.9.0)

##### Compile Details ####
option(ARCH_WIN64 "Build for 64 bit." ON)
option(USE_D3D12 "Build with dx12 impl." ON)
#option(USE_VULKAN "Build with vk impl." OFF)
option(RLE_BUILD_TESTS "Build engine tests." ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

##### COMPILER SETTINGS #####
set (CMAKE_CXX_STANDARD 17)
message(STATUS "Using C++"  ${CMAKE_CXX_STANDARD})

##### OUTPUT DIRECTORIES #####
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

##### OPTIONS #####
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")

set(BUILD_STATIC_LIB ON)
set(NO_EXPORT ON)

##### DEPENDENCIES #####
add_subdirectory(deps/glfw)
add_subdirectory(deps/assimp)

##### STRUCTURE DEPENDENCIES #####
set_target_properties(zlib PROPERTIES FOLDER "Dependencies/")
set_target_properties(glfw PROPERTIES FOLDER "Dependencies/GLFW")
set_target_properties(uninstall PROPERTIES FOLDER "Dependencies/GLFW")
set_target_properties(assimp PROPERTIES FOLDER "Dependencies/Assimp")
set_target_properties(assimp_cmd PROPERTIES FOLDER "Dependencies/Assimp")
set_target_properties(IrrXML PROPERTIES FOLDER "Dependencies/Assimp")
set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "Dependencies/Assimp")
set_target_properties(zlibstatic PROPERTIES FOLDER "Dependencies/Assimp")
set_target_properties(unit PROPERTIES FOLDER "Dependencies/Assimp")

##### DEFINITIONS #####
#add_definitions(-DENABLE_GRAPHICS_VALIDATION_LAYERS)
#message(STATUS "Renderer: Using graphics API validation layers")

##### INCLUDE/LINK DIRECTORIES #####
# 64 BIT
if (ARCH_WIN64)
	link_directories(
		#C:/VulkanSDK/1.0.57.0/Lib
		${CMAKE_BINARY_DIR}/lib/
		${CMAKE_BINARY_DIR}/deps/glfw/src
	)
else()
	link_directories(
		#C:/VulkanSDK/1.0.57.0/Lib32
		${CMAKE_BINARY_DIR}/lib/
		${CMAKE_BINARY_DIR}/deps/glfw/src
	)
endif()

include_directories(
	C:/VulkanSDK/1.0.57.0/Include
	modules/
	deps/assimp/include/
	deps/glfw/include/
)

##### SRC/TESTS SUB-DIRECTORIES #####
add_subdirectory(src)

if (RLE_BUILD_TESTS)
	add_subdirectory(tests)

	### STARTUP ###
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RenderTest)
endif()

