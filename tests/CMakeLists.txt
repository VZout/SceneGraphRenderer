cmake_minimum_required(VERSION 3.9.0)

function(add_test EXAMPLE_NAME EXAMPLE_LIBS)
	add_definitions(-DUSE_D3D12)
	SET(EXAMPLE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME})
	message(STATUS "Configuring example ${EXAMPLE_NAME}")

	# source
	file(GLOB SOURCES "${EXAMPLE_NAME}/*.cpp")
	file(GLOB HEADERS "${EXAMPLE_NAME}/*.h")

	add_executable(${EXAMPLE_NAME} ${SOURCES} ${HEADERS})
	target_include_directories(${EXAMPLE_NAME} PUBLIC ../src)
	add_dependencies(${EXAMPLE_NAME} ${EXAMPLE_NAME})
	target_link_libraries(${EXAMPLE_NAME} ${EXAMPLE_LIBS})
	set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER "Tests")
	set_target_properties(${EXAMPLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/../")
endfunction(add_test)

add_test(BasicRenderingTest RLRenderer)
add_test(ECSTest RLRenderer RLEngine)
