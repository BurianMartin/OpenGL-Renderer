cmake_minimum_required(VERSION 3.16)
project(OpenGL_App VERSION 1.0.0 LANGUAGES CXX)

enable_language(C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_source_files_properties(src/glad/gl.c PROPERTIES LANGUAGE C)

include(GNUInstallDirs)

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

add_library(Forge::EngineCore ALIAS EngineCore)

# BUILD_INTERFACE/INSTALL_INTERFACE split so this target is exportable — a plain absolute
# ${CMAKE_SOURCE_DIR} path can't appear in an install(EXPORT ...) targets file (CMake errors
# on it outright), and a consumer linking the installed package needs the install prefix's
# own include dir, not this build tree's.
target_include_directories(EngineCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/glad>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/glad>
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

# --- Install (SDL2/GLFW-style: `cmake --install build --prefix <dir>` makes Forge consumable
# from a separate project via `find_package(Forge REQUIRED)` + `target_link_libraries(... Forge::EngineCore)`) ---
install(TARGETS EngineCore
    EXPORT ForgeTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Excludes include/Demo/ deliberately -- those headers' matching .cpp files only compile
# into OpenGL_App, not EngineCore, so they have no symbols in the installed static library;
# installing the headers without them would produce link errors for anyone who tried to use
# Demo:: classes against the installed package. Demo is app-specific example code, not part
# of the reusable engine (see CLAUDE.md's Forge/Demo namespace split) -- a new consumer
# project doesn't want or need it.
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PATTERN "Demo" EXCLUDE
)

# Core engine assets Forge itself loads via CWD-relative paths at runtime — not demo content:
# the shared vertex shader, DebugOverlayLayer's unlit overlay fragment shader, Text's shader
# pair, and the default font DebugOverlayLayer/demo code load by that exact relative path.
# Forge resolves resource paths relative to the running process's working directory by design
# (see CLAUDE.md's "always run from project root"), so these are installed as reference copies,
# not auto-discovered by the installed library at runtime — a new consumer project needs its
# own copies (or symlinks) of these specific files in ITS working directory, at these same
# relative paths (shaders/..., fonts/...), for any Forge feature that loads them by name to work.
install(FILES
    shaders/vertex.glsl
    shaders/debug_overlay_fragment.glsl
    shaders/text_vertex.glsl
    shaders/text_fragment.glsl
    DESTINATION ${CMAKE_INSTALL_DATADIR}/forge/shaders
)
install(FILES
    fonts/DejaVuSans.ttf
    fonts/DejaVuSans-LICENSE.txt
    DESTINATION ${CMAKE_INSTALL_DATADIR}/forge/fonts
)

install(EXPORT ForgeTargets
    FILE ForgeTargets.cmake
    NAMESPACE Forge::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Forge
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ForgeConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ForgeConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Forge
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ForgeConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ForgeConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ForgeConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Forge
)
