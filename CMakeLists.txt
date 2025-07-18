cmake_minimum_required(VERSION 3.25.0)
project(DOCXEngineOpenGL VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/src/resources)
set(PASSES_DIR ${CMAKE_SOURCE_DIR}/src/passes)

set (
	IMGUI_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/ImGui/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/ImGui/imgui_draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/ImGui/imgui_widgets.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/ImGui/imgui_impl_glfw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/ImGui/imgui_impl_opengl3.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/ImGui/imgui_tables.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/ImGui/imgui_demo.cpp
	)


add_subdirectory(${PASSES_DIR})
add_subdirectory(${RESOURCE_DIR})

add_library(glad STATIC thirdparty/include/glad/glad.c)
target_include_directories(glad PUBLIC thirdparty/include)

add_executable(DOCXEngineOpenGL 
	src/main.cpp 
	src/camera.cpp
	src/stb_image_impl.cpp
	src/tiny_gltf_impl.cpp
	src/uiManager.cpp
	src/sceneManager.cpp
	src/appConfig.cpp
	src/inputManager.cpp
	src/renderer.cpp
	${IMGUI_SRC}
)

target_include_directories(DOCXEngineOpenGL PRIVATE 
	${CMAKE_SOURCE_DIR}/thirdparty/include
	${CMAKE_SOURCE_DIR}/src
	${PASSES_DIR}
	${RESOURCE_DIR}
)

target_link_directories(DOCXEngineOpenGL PRIVATE 
	${CMAKE_SOURCE_DIR}/thirdparty/lib
	${CMAKE_SOURCE_DIR}/thirdparty/include
	${CMAKE_SOURCE_DIR}/src
	${PASSES_DIR}
	${RESOURCE_DIR}

)

target_link_libraries(DOCXEngineOpenGL PRIVATE
	glad
	glfw3
	opengl32
	passes
	resources
)