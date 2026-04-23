cmake_minimum_required(VERSION 3.16)
project(OpenGL_App LANGUAGES CXX)

enable_language(C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_source_files_properties(src/glad/gl.c PROPERTIES LANGUAGE C)

find_package(glfw3 3.4 REQUIRED)
find_package(glm REQUIRED)

add_executable(OpenGL_App 
        src/main.cpp
        src/glad/gl.c  
        src/Core/Scene.cpp
        src/Core/Model.cpp
        src/Core/Shader.cpp
        src/Core/Renderer.cpp
        src/App/Application.cpp
    ) 

target_include_directories(OpenGL_App
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/Core
        ${CMAKE_SOURCE_DIR}/include/App
        ${CMAKE_SOURCE_DIR}/src/App
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include/glad 
)

target_link_libraries(OpenGL_App PRIVATE glfw GL glm::glm)