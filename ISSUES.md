# Issues Checklist

## Bugs (wrong behaviour / crashes)

- [x] **Camera rotates in free-cursor mode** — fixed: `Application::RaiseEvent` now filters `MouseMoved` events when `cursor_mode_ == GLFW_CURSOR_NORMAL`. Default cursor mode changed to `GLFW_CURSOR_DISABLED`; press backtick to toggle to free-cursor mode.

- [x] **Camera jumps when re-enabling cursor capture** — fixed: `Camera::ResetMouseTracking()` sets `first_mouse_ = true`; called via `Scene::OnMouseCaptured()` which `Application` invokes when toggling to `GLFW_CURSOR_DISABLED`.

- [x] **`WindowResizeEvent` is never fired** — fixed: callback uncommented in `EventHandler.cpp`; `Scene::HandleEvent` updates `rctx_->aspect_ratio_` and camera aspect ratio on resize; `Application::RaiseEvent` calls `glViewport` to expand the draw area.

- [ ] **Vertex normals and texCoords uploaded but unused** — `Mesh::setup()` wires vertex attributes at locations 0 (position), 1 (normal), 2 (texCoords). `vertex.glsl` only declares `layout(location = 0) in vec3 aPos`, so normals and texCoords are silently discarded by the driver. Any lighting or UV work will require updating the vertex shader.

- [x] **TestLayer: both models rendered at origin** — fixed: single model scaled to 0.05 and positioned to center the OBJ at world origin; second model removed.

- [x] **`Init()` failure doesn't abort the constructor** — fixed.

---

## Design / Architecture Issues

- [x] **`Application` constructor hardcodes scene setup** — fixed: scene construction moved to `main.cpp`; `Application` no longer depends on any app-layer type.

- [x] **`Application.hpp` includes app-layer headers** — fixed: `App/TestLayer.hpp` and `App/TestScene.hpp` removed from `Application.hpp`.

- [x] **`rctx_->aspect_ratio_` was garbage on startup** — fixed: `specification_` was declared after `renderer_` in `Application`, so `renderer_` was constructed from uninitialized width/height. Fixed by reordering member declarations so `specification_` initializes first.

- [x] **`WindowSpecification::VSync` is silently ignored** — fixed: `Application::Init` now branches on `VSync`; also fixed call-order bug where `glfwSwapInterval` was called before `glfwMakeContextCurrent` (making it a no-op regardless of the value).

- [x] **`EventHandler` default constructor leaves `window_` uninitialised** — fixed: default constructor removed; only the parameterized constructor exists now.

- [x] **`main.cpp` calls `App.Destroy()` explicitly before destructor** — fixed: redundant explicit call removed.

---

## Code Quality / Correctness Smells

- [x] **Shader uniform locations fetched on every call** — fixed: private `GetUniformLocation(name)` helper caches results in `unordered_map<string, GLint>` on first use; all setters use it instead of calling the driver directly.

- [x] **`glUseProgram(shader->ID)` bypasses `Shader::Use()`** — fixed: `TestLayer::OnEvent` now calls `shader->Use()`.

- [x] **`DEBUG` macro is unconditionally defined** — fixed: `Utils.hpp` now uses `#ifndef NDEBUG`; debug macros are no-ops in Release builds automatically.

- [x] **`EventHandler::RaiseEvent` logs every event** — fixed: replaced `debug_info` with a new `debug_event` macro (cyan output) gated on `LOG_EVENTS`. Enable with `cmake … -DLOG_EVENTS=ON`; off by default so normal debug builds are clean.

- [x] **Double color-buffer clear per frame** — fixed: `Renderer::RenderScene` now only clears `GL_DEPTH_BUFFER_BIT`; color clear is owned entirely by `DrawSolidBackground`.

- [x] **`debug_info` fires every frame for scenes without a background** — fixed: removed `debug_info` call from the `None` case in `Scene::DrawBackground`.

- [x] **`return; break;` dead code in `Scene::HandleEvent`** — fixed: unreachable `break` statements after `return` removed from MouseMoved, MouseScrolled, and WindowResize cases.

- [x] **`RenderContext.hpp` uses `GLfloat` without including glad** — fixed: `#include <glad/gl.h>` added directly to the header.

---

## Bugs introduced in the Core/App Scene refactor (current)

- [x] **`OnSceneBoot()` is never called — segfault** — Fixed: `Core::Scene::OnLoad` now calls `OnSceneBoot()` after storing the shared_ptrs.

- [x] **No camera is ever created — segfault** — Fixed: `TestScene::OnSceneBoot` calls `cameras_.emplace_back(rctx_->aspect_ratio_)` before adding layers.

- [x] **`key_map` declared `static const` in a header** — fixed: changed to `inline const` in `include/Core/Camera.hpp`.

- [x] **Duplicate `Background_Type` enum in `TestScene.hpp`** — fixed: removed from `include/App/TestScene.hpp`.

- [x] **Wrong namespace closing comment in `TestScene.cpp`** — fixed: now correctly says `// namespace Test`.

---

## Next Additions (planned, not yet started)

- [x] **`AppContext` / `AcceptScene` / `OnAddedToApp`** — superseded: the refactor went a different direction. `Application::AddScene` now passes `rmanager` and `rctx` directly into `Scene::OnLoad(rmanager, rctx)`, which then calls `OnSceneBoot()`. The scene owns its own initialization sequence.

- [x] **Make `Scene` abstract** — done: `OnEvent`, `OnUpdate`, `OnMouseCapture`, and `OnSceneBoot` are all pure virtual.

- [ ] **Wire normals / texCoords in vertex shader** — add `layout(location = 1) in vec3 aNormal` and `layout(location = 2) in vec2 aTexCoords` to `vertex.glsl`, pass them through to fragment shaders as `out` varyings. **Prerequisite for everything below.**

- [ ] **`Texture` class** — load image data via stb_image, upload to GPU (`glTexImage2D`), expose `Bind(slot)` / `Unbind()`. Factory pattern `Texture::Create(path)` consistent with `Mesh` and `Shader`. Integrate into `ResourceManager::LoadTexture(path)` for deduplication via `weak_ptr` cache.

- [ ] **`Material` class** — owns a `Shader` + one or more `Texture`s + per-material uniform values (base color, roughness, etc.). Exposes `Bind()` which activates the shader, binds textures to their slots, and sets uniforms. `Model` gains a `material_` member. `Layer::OnRender` iterates by material instead of by shader (replaces the current `shaderModels_` map with a `materialModels_` equivalent).

- [ ] **Lighting** — depends on normals in vertex shader. Minimum viable: directional light + point light, Blinn-Phong in the fragment shader. Needs a `Light` struct (position/direction, color, intensity, attenuation for point lights). Scene holds a list of lights; they are passed to shaders either as uniforms or via a UBO. This is the largest piece of work here.

- [ ] **Multiple cameras / split-screen** — `Scene::cameras_` and `active_camera_` already support multiple cameras; the render side does not. Needs: a viewport rect per camera, `glViewport` calls to partition the window, and `Renderer::RenderScene` running once per active camera per frame with its own view/projection. `RenderContext` gets updated per camera per pass. Independent of lighting but architecturally significant.