# Issues Checklist

## Bugs (wrong behaviour / crashes)

- [ ] **Camera rotates in free-cursor mode** — when the cursor is switched to `GLFW_CURSOR_NORMAL` (backtick toggle), `ProcessMousePosition` is still called on every mouse-move event. `first_mouse_` should be reset on cursor mode change, and the scene should skip forwarding `MouseMoved` events when the cursor is not captured.

---

## Design / Architecture Issues

- [ ] **`Application` constructor hardcodes scene/layer setup** — `Application.cpp:9-16` directly constructs `TestLayer` and a `Scene` inside the engine constructor. Scenes and layers should be built outside and passed in, or the engine should expose an `OnInit()` virtual/callback for subclasses.

- [ ] **No central resource/state holder** — shaders, meshes, and global state (e.g. window size, aspect ratio) are currently scattered. A `ResourceManager` or `ApplicationContext` struct passed by reference would give layers and scenes a single place to look up shared resources without being coupled directly to `Application`. Linked to the aspect ratio issue below. See [[scene-aspect-ratio]].

- [ ] **Scene receives aspect ratio as a baked-in float** — `Scene(float aspect_ratio)` bakes the ratio at construction; it never updates on window resize and has no connection to the actual window. Options: (a) pass a `const WindowSpecification&` reference so Scene/Camera always read current width/height, or (b) source it from the `ResourceManager` once that exists. See [[no-resource-manager]].

---

## Code Quality / Correctness Smells

- [ ] **Shader uniform locations fetched on every call** — every `SetVec4`, `SetMat4`, etc. calls `glGetUniformLocation` each invocation. Cache results in a `std::unordered_map<std::string, GLint>` populated on first use.

---

## Next Additions (planned, not yet started)

- [ ] **`AppContext` / `AcceptScene` / `OnAddedToApp`** — replace the current `Scene(float aspect_ratio)` constructor pattern so `Application` is the single source of truth for window data.

  **Steps:**
  1. Add `AppContext` struct to `include/Core/Specifications.hpp`:
     ```cpp
     struct AppContext {
         float aspect_ratio;
         // grows into a ResourceManager over time
     };
     ```
  2. Replace `AddScene` → `AcceptScene` in `Application`: build an `AppContext`, call `scene->OnAddedToApp(ctx)`, then push to `scenes_`.
  3. Add `Scene::OnAddedToApp(const AppContext&)` — sets aspect ratio on all owned cameras. Remove `float aspect_ratio` from the `Scene` constructor.
  4. Live resize: handle `WindowResizeEvent` inside `Scene::HandleEvent` → call `camera.SetAspectRatio(new_ratio)` on all cameras.

  **Before starting — verify these two things:**
  - Does `EventHandler` actually fire `WindowResizeEvent` on the GLFW framebuffer/window resize callback? (`EventType::WindowResize` exists in the enum but wiring is unconfirmed.)
  - `Camera` needs a `SetAspectRatio(float)` setter.
