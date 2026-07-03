cmake_minimum_required(VERSION 3.16)
project(OpenGL_App LANGUAGES CXX)

enable_language(C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_source_files_properties(src/glad/gl.c PROPERTIES LANGUAGE C)

find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

add_executable(OpenGL_App 
        src/main.cpp
        src/glad/gl.c

        src/App/TestLayer.cpp
        src/App/TestScene.cpp

        src/Core/Mesh.cpp
        src/Core/Model.cpp
        src/Core/Camera.cpp
        src/Core/Shader.cpp
        src/Core/Texture.cpp
        src/Core/Material.cpp
        src/Core/Colors.cpp
        src/Core/Renderer.cpp
        src/Core/Application.cpp
        src/Core/EventHandler.cpp
        src/Core/ResourceManager.cpp
        src/Core/Scene.cpp
        src/Core/Lights.cpp
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

option(SHOW_FPS "Show FPS counter in console" OFF)
if(SHOW_FPS)
    target_compile_definitions(OpenGL_App PRIVATE SHOW_FPS)
endif()

option(LOG_EVENTS "Log all input events to console" OFF)
if(LOG_EVENTS)
    target_compile_definitions(OpenGL_App PRIVATE LOG_EVENTS)
endif()
 