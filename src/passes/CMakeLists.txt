# Resources CMakeLists.txt
add_library(passes STATIC)

# Gather all source files
file(GLOB RESOURCES_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

target_sources(passes
    PRIVATE
        ${RESOURCES_SOURCES}
)

target_include_directories(passes
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_SOURCE_DIR}/thirdparty/include
		${CMAKE_SOURCE_DIR}/src
		${CMAKE_SOURCE_DIR}/src/resources
)

# Link against required dependencies
target_link_libraries(passes
    PUBLIC
	glfw3
	opengl32
)

# Set C++ standard
set_target_properties(passes PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)