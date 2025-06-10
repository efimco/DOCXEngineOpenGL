# Resources CMakeLists.txt
add_library(resources STATIC)

# Gather all source files
file(GLOB RESOURCES_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
file(GLOB SCENE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/scene/*.cpp" 
)

target_sources(resources
    PRIVATE
        ${RESOURCES_SOURCES}
        ${SCENE_SOURCES}
)

target_include_directories(resources
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_SOURCE_DIR}/thirdparty/include
		${CMAKE_SOURCE_DIR}/src
		${CMAKE_SOURCE_DIR}/src/passes
)

# Link against required dependencies
target_link_libraries(resources
    PUBLIC
	glfw3
	opengl32
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/default_imgui.ini
    ${CMAKE_BINARY_DIR}/Debug/resources/default_imgui.ini
    COPYONLY
)

# Set C++ standardPRIVATE
set_target_properties(resources PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)