cmake_minimum_required(VERSION 3.16)
project(OpenGL_App LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 3.4 REQUIRED)

add_executable(OpenGL_App 
        src/main.cpp
        src/App/Application.cpp
    ) 

target_include_directories(OpenGL_App
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/App
        ${CMAKE_SOURCE_DIR}/src/App
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(OpenGL_App PRIVATE glfw GL)