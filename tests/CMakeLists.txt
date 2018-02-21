cmake_minimum_required(VERSION 3.9.0)

function(buildTest EXAMPLE_NAME)
	add_definitions(-DUSE_D3D12)
	SET(EXAMPLE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME})
	message(STATUS "Configuring example ${EXAMPLE_NAME}")

	# source
	file(GLOB SOURCES "${EXAMPLE_NAME}/*.cpp")
	file(GLOB HEADERS "${EXAMPLE_NAME}/*.h")

	add_executable(${EXAMPLE_NAME} ${SOURCES} ${HEADERS})
	target_include_directories(${EXAMPLE_NAME} PUBLIC ../src)
	add_dependencies(${EXAMPLE_NAME} RLCommon RLRenderer RLEngine)
	target_link_libraries(${EXAMPLE_NAME} RLCommon RLRenderer RLEngine glfw3)
	set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER "Tests")
	set_target_properties(${EXAMPLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/../")
endfunction(buildTest)

function(buildTests)
	foreach(EXAMPLE ${EXAMPLES})
		buildTest(${EXAMPLE})
	endforeach(EXAMPLE)
endfunction(buildTests)

# Modify this list to add your own tests.
set(EXAMPLES
	BasicRenderingTest ECSTest
)

buildTests()
