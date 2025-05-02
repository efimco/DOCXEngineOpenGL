cmake_minimum_required(VERSION 3.5.0)
project(LearnOpenGL VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


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


add_executable(OpenGLLearn 
	thirdparty/include/glad/glad.c
	src/main.cpp 
	src/camera.cpp
	src/texture.cpp
	src/material.cpp
	src/shader.cpp
	src/primitive.cpp
	src/gltfImporter.cpp
	src/stb_image_impl.cpp
	src/tiny_gltf_impl.cpp
	src/shadowMap.cpp
	src/pickingBuffer.cpp
	src/cubemap.cpp
	src/uiManager.cpp
	src/sceneManager.cpp
	src/appConfig.cpp
	src/inputManager.cpp
	src/renderer.cpp
	${IMGUI_SRC}
)

target_include_directories(OpenGLLearn PRIVATE 
	thirdparty/include
)

target_link_directories(OpenGLLearn PRIVATE 
	thirdparty/lib
)

target_link_libraries(OpenGLLearn PRIVATE 
	glfw3
	opengl32
)