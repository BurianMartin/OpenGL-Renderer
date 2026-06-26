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

- [ ] **`Application` constructor hardcodes scene setup** — `Application.cpp:12-13` directly constructs `TestScene` inside the engine constructor. Scenes should be built outside and passed in, or the engine should expose an `OnInit()` virtual/callback for subclasses.

- [ ] **`Application.hpp` includes app-layer headers** — `Application.hpp:11-12` includes `App/TestLayer.hpp` and `App/TestScene.hpp`, creating a compile-time dependency from the engine core on app-specific code. These should be removed once the hardcoded scene setup is extracted.

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

## Next Additions (planned, not yet started)

- [ ] **`AppContext` / `AcceptScene` / `OnAddedToApp`** — replace the current `Scene(ResourceManager)` + `AddScene` pattern so `Application` is the single source of truth for window data.

  **Steps:**
  1. Add `AppContext` struct to `include/Core/Specifications.hpp`:
     ```cpp
     struct AppContext {
         float aspect_ratio;
         std::shared_ptr<ResourceManager> resources;
     };
     ```
  2. Replace `AddScene` → `AcceptScene` in `Application`: build an `AppContext`, call `scene->OnAddedToApp(ctx)`, then push to `scenes_`.
  3. Add `Scene::OnAddedToApp(const AppContext&)` — sets aspect ratio on all owned cameras. Remove `ResourceManager` from the `Scene` constructor (pass via context instead).
  4. Live resize: wire `WindowResizeEvent` in `EventHandler`, handle it in `Scene::HandleEvent` → call `camera.SetAspectRatio(new_ratio)` on all cameras. Requires adding `Camera::SetAspectRatio(float)`.

- [ ] **Make `Scene` abstract** — enforce subclassing so users can't accidentally instantiate the base class directly. Pairs with the `AppContext`/`AcceptScene` refactor and the plan to make this a proper engine API.

- [ ] **Wire normals / texCoords in vertex shader** — add `layout(location = 1) in vec3 aNormal` and `layout(location = 2) in vec2 aTexCoords` to `vertex.glsl`, pass them through to fragment shaders as `out` varyings. Required before lighting or texture work can begin.