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
    )


add_executable(OpenGLLearn 
    src/main.cpp 
    src/glad.c
    src/shader.h
    src/shader.cpp
    src/objReader.h
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