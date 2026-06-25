# Issues Checklist

## Bugs (wrong behaviour / crashes)

- [x] **Camera rotates in free-cursor mode** — fixed: `Application::RaiseEvent` now filters `MouseMoved` events when `cursor_mode_ == GLFW_CURSOR_NORMAL`. Default cursor mode changed to `GLFW_CURSOR_NORMAL`; press backtick to capture cursor for camera control.

- [ ] **`WindowResizeEvent` is never fired** — `EventHandler.cpp:38-44` registers the GLFW window size callback but the `WindowResizeEvent` creation is commented out. As a result, `rctx_->aspect_ratio_` is never updated after startup — zooming or resizing the window distorts the projection.

- [ ] **Vertex normals and texCoords uploaded but unused** — `Mesh::setup()` wires vertex attributes at locations 0 (position), 1 (normal), 2 (texCoords). `vertex.glsl` only declares `layout(location = 0) in vec3 aPos`, so normals and texCoords are silently discarded by the driver. Any lighting or UV work will require updating the vertex shader.

- [x] **TestLayer: both models rendered at origin** — fixed: `model` scaled to 0.05 and positioned to center the OBJ at world origin; `model2` offset to (1.05, 0.45, -3.25).

---

## Design / Architecture Issues

- [ ] **`Application` constructor hardcodes scene setup** — `Application.cpp:12-13` directly constructs `TestScene` inside the engine constructor. Scenes should be built outside and passed in, or the engine should expose an `OnInit()` virtual/callback for subclasses.

- [x] **`rctx_->aspect_ratio_` was garbage on startup** — fixed: `specification_` was declared after `renderer_` in `Application`, so `renderer_` was constructed from uninitialized width/height. Fixed by reordering member declarations so `specification_` initializes first. Remaining: aspect ratio is still baked at startup and won't update on window resize until `WindowResizeEvent` is wired.

---

## Code Quality / Correctness Smells

- [ ] **Shader uniform locations fetched on every call** — every `SetVec4`, `SetMat4`, etc. calls `glGetUniformLocation` each invocation. Cache results in a `std::unordered_map<std::string, GLint>` populated on first use.

- [x] **Double color-buffer clear per frame** — fixed: `Renderer::RenderScene` now only clears `GL_DEPTH_BUFFER_BIT`; color clear is owned entirely by `DrawSolidBackground`.

- [x] **`debug_info` fires every frame for scenes without a background** — fixed: removed `debug_info` call from the `None` case in `Scene::DrawBackground`.

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

- [ ] **Wire normals / texCoords in vertex shader** — add `layout(location = 1) in vec3 aNormal` and `layout(location = 2) in vec2 aTexCoords` to `vertex.glsl`, pass them through to fragment shaders as `out` varyings. Required before lighting or texture work can begin.
