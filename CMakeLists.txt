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

# --- EngineCore: the reusable framework/engine layer ---
# Everything under src/Core (+ glad, which the engine depends on) lives here so it can be
# linked by OpenGL_App, future engine consumers, and the test binary alike, without pulling
# in main.cpp or app-layer (src/App) code.
add_library(EngineCore STATIC
        src/glad/gl.c

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

target_include_directories(EngineCore
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/Core
        ${CMAKE_SOURCE_DIR}/include/glad
)

# PUBLIC: Core headers expose GLFW/glm types (e.g. Application.hpp, EventHandler.hpp), so
# anything linking EngineCore needs these propagated too, not just EngineCore's own .cpp files.
target_link_libraries(EngineCore PUBLIC glfw GL glm::glm)

option(SHOW_FPS "Show FPS counter in console" OFF)
if(SHOW_FPS)
    target_compile_definitions(EngineCore PUBLIC SHOW_FPS)
endif()

option(LOG_EVENTS "Log all input events to console" OFF)
if(LOG_EVENTS)
    target_compile_definitions(EngineCore PUBLIC LOG_EVENTS)
endif()

# --- OpenGL_App: the demo application layer (src/App + main.cpp) ---
add_executable(OpenGL_App
        src/main.cpp

        src/App/TestLayer.cpp
        src/App/TestScene.cpp
    )

target_include_directories(OpenGL_App
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include/App
        ${CMAKE_SOURCE_DIR}/src/App
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(OpenGL_App PRIVATE EngineCore)

# --- Tests (opt-in: -DBUILD_TESTS=ON) ---
option(BUILD_TESTS "Build the EngineCore test suite" OFF)
if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()

    add_executable(EngineCore_tests
        test/test_camera.cpp
        test/test_mesh.cpp
        test/test_lights.cpp
        test/test_model.cpp
        test/test_colors.cpp
    )
    target_link_libraries(EngineCore_tests PRIVATE EngineCore GTest::gtest GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(EngineCore_tests)
endif()
