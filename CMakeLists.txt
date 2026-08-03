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
# Everything under src/Forge (+ glad, which the engine depends on) lives here so it can be
# linked by OpenGL_App, future engine consumers, and the test binary alike, without pulling
# in main.cpp or demo-layer (src/Demo) code.
add_library(EngineCore STATIC
        src/glad/gl.c

        # Core: engine bootstrap, GLFW event wiring, resource loading
        src/Forge/Core/Engine.cpp
        src/Forge/Core/EventHandler.cpp
        src/Forge/Core/ResourceManager.cpp
        src/Forge/Core/Renderer.cpp

        # Scene: the Engine -> Scene -> Layer hierarchy + per-frame data
        src/Forge/Scene/Scene.cpp
        src/Forge/Scene/DebugOverlayLayer.cpp

        # Rendering: geometry, transforms, camera, materials/shaders/textures, picking, text
        src/Forge/Rendering/Mesh.cpp
        src/Forge/Rendering/Model.cpp
        src/Forge/Rendering/Camera.cpp
        src/Forge/Rendering/Shader.cpp
        src/Forge/Rendering/Texture.cpp
        src/Forge/Rendering/Material.cpp
        src/Forge/Rendering/Colors.cpp
        src/Forge/Rendering/Font.cpp
        src/Forge/Rendering/Text.cpp

        # Lighting
        src/Forge/Lighting/Lights.cpp
    )

target_include_directories(EngineCore
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/glad
)

# PUBLIC: Forge headers expose GLFW/glm types (e.g. Engine.hpp, EventHandler.hpp), so
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

# --- OpenGL_App: the demo application layer (src/Demo + main.cpp) ---
# Opt-out (default ON, matching the historical always-built behavior): -DBUILD_DEMO=OFF
# builds EngineCore alone, with no demo-layer code compiled at all — the "just the library"
# path for an external consumer project. Sticky like SHOW_FPS/LOG_EVENTS until toggled or
# `make clean`.
option(BUILD_DEMO "Build the Forge demo app (src/Demo + main.cpp)" ON)
if(BUILD_DEMO)
    add_executable(OpenGL_App
            src/main.cpp

            src/Demo/LightDemoLayer.cpp
            src/Demo/LightDemoScene.cpp
            src/Demo/MultiCameraDemoScene.cpp
            src/Demo/OrthoDemoScene.cpp
        )

    target_include_directories(OpenGL_App
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include/Demo
            ${CMAKE_SOURCE_DIR}/src/Demo
            ${CMAKE_SOURCE_DIR}/src
    )

    target_link_libraries(OpenGL_App PRIVATE EngineCore)
endif()

# --- Tests (opt-in: -DBUILD_TESTS=ON) ---
option(BUILD_TESTS "Build the EngineCore test suite" OFF)
if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()

    add_executable(EngineCore_tests
        test/test_camera.cpp
        test/test_viewport.cpp
        test/test_mesh.cpp
        test/test_lights.cpp
        test/test_model.cpp
        test/test_tweens.cpp
        test/test_prop.cpp
        test/test_colors.cpp
        test/test_keys.cpp
        test/test_clickable_region.cpp
    )
    target_link_libraries(EngineCore_tests PRIVATE EngineCore GTest::gtest GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(EngineCore_tests)
endif()
