# Issues Checklist

## Bugs (wrong behaviour / crashes)

- [ ] **`Model` rotation is broken** — `rotation_` is declared as `glm::quat` in `Model.hpp` but in `getModelMatrix()` (`Model.cpp:21-23`) its `.x/.y/.z` components are passed as angles to `glm::rotate`. Those fields are the quaternion's i/j/k components, not Euler angles. Fix: either change `rotation_` to `glm::vec3` (Euler), or keep quat and replace the three `glm::rotate` calls with `glm::mat4_cast(rotation_)`.

- [x] **`Mesh` destructor leaks GPU resources** — fixed: destructor now calls `destroy()`.

- [ ] **`Mesh` move constructor/assignment declared but not implemented** — `Mesh.hpp:37-38` declares both but `Mesh.cpp` has no definitions. Any attempt to move a `Mesh` will cause a linker error. The move constructor needs to steal the GPU handles and zero them out on the source; the destructor then safely no-ops on a zeroed handle.

- [ ] **Depth test never enabled** — `glEnable(GL_DEPTH_TEST)` is never called anywhere. The depth buffer is cleared every frame but doesn't influence rendering, so triangles draw in submission order rather than by depth. 3D scenes will have incorrect overlap.

- [ ] **`Scene::Update` never calls `layer->OnUpdate()`** — `Scene.cpp:29-32` only calls `cameras_[active_camera_].Update(delta_time)`. All layers' `OnUpdate()` methods silently never fire.

- [ ] **Camera rotates in free-cursor mode** — when the cursor is switched to `GLFW_CURSOR_NORMAL` (backtick toggle), `ProcessMousePosition` is still called on every mouse-move event. The camera rotates even though the cursor is free. `first_mouse_` should be reset on cursor mode change, and the scene should skip forwarding `MouseMoved` events when the cursor is not captured.

- [ ] **`RandomColor()` always returns the same sequence** — `Colors.cpp:5` calls `rand()` with no `srand()` seed, so it produces an identical sequence every run. Seed with `srand(time(nullptr))` once at startup, or replace with a `<random>` engine.

- [ ] **Shader compile/link errors are `debug_warn`, not `debug_error`** — `Shader.cpp:103,113`. A failed shader or program leaves `ID` in an undefined state and the app continues silently rendering nothing (or crashing on uniform calls). These should be fatal.

- [ ] **Mouse scroll never reaches `Camera::Zoom`** — `Camera::Zoom` exists but nothing calls it. `Scene::HandleEvent` forwards `MouseScrolledEvent` to layers, but no layer calls `Zoom`. The scroll callback fires correctly, the event just goes nowhere useful.

- [ ] **`Mesh` `drawMode` parameter silently ignored** — both constructors (`Mesh.cpp:5`, `Mesh.cpp:11`) accept a `drawMode` argument but never assign it to the member before calling `setup()`. The member is default-initialised to `GL_TRIANGLES` in the class definition, so passing `GL_LINES` or `GL_POINTS` has no effect.

- [ ] **`TestLayer::OnEvent` fall-through bug** — `TestLayer.cpp:99`. The outer `switch` on `GetEventType()` has a `case KeyPressed:` block that ends without a `break` or `return` after the inner switch. Execution falls through into `case MouseButtonPressed:` which unconditionally returns `true`. Any key event that reaches the default inner case (anything other than Tab) will silently trigger the mouse-button path.

- [ ] **`std::ranges::find` on `shaders_` crashes if tag not found** — `TestLayer.cpp:112`. The iterator returned by `std::ranges::find` is immediately dereferenced with `*`. If no shader with the tag `"Solid"` exists, this dereferences `shaders_.end()` — undefined behaviour. The result should be checked before dereferencing.

- [ ] **`Application::AddScene` always resets `current_scene_` to `0`** — `Application.cpp:94-97`. Every call to `AddScene` sets `current_scene_ = 0`, so adding a second scene immediately makes it impossible to switch to the first. The assignment should only set `current_scene_` if no scene is active yet.

- [ ] **`Application::Destroy()` does not call `glfwTerminate()`** — `Application.cpp:85-91`. GLFW is initialised in `Init()` but never terminated. Additionally, `~Application()` is defaulted and does not call `Destroy()`, so if the destructor runs (e.g. on an exception) neither layers nor GLFW are cleaned up.

- [ ] **`WindowCloseEvent` is effectively dead code** — `EventHandler.cpp:68-73` raises it, but `Application::RaiseEvent` has no handler for `EventType::WindowClose`. The window closes only because GLFW sets the close flag internally before the callback fires. The event is never consumed by the engine.

- [ ] **`Event::handled_` is declared but never read or written** — `Event.hpp:33`. The `bool handled_` field is initialised to `false` and never touched anywhere. The current consumption mechanism is the `bool` return from `OnEvent`, making this field misleading dead state.

- [ ] **`RandomColor()` can never return `Lime`, `SkyBlue`, or `Brown`** — `Colors.hpp:25-27` defines three colours not reachable from `Colors.cpp:5`. The switch covers cases 0–11 (12 values) but 15 colours exist. The three newest colours are permanently unreachable.

- [ ] **`Scene::DrawBackground()` spams `debug_warn` every frame** — `Scene.cpp:69`. When `backgroundType_` is `None`, `debug_warn("No background set.")` fires on every render call, flooding stdout at frame rate.

- [ ] **`Camera::MoveUp` logs every frame** — `Camera.cpp:50`. A leftover `debug_info("CameraMove: UP true")` fires every frame the camera moves upward. No other direction has this.

- [ ] **`EventHandler` raw pointer in GLFW can dangle** — `EventHandler.cpp:8` stores `this` (a raw `EventHandler*`) in the GLFW window user pointer. The owning `shared_ptr<EventHandler>` lives in `Application`. If `Application` is destroyed while the GLFW window still exists and a callback fires, the raw pointer is dangling. GLFW must be shut down before the `EventHandler` is destroyed.

---

## Design / Architecture Issues

- [ ] **Two independent delta-time calculations** — `Application::Run()` (`Application.cpp:73-75`) computes `delta_time` and passes it to `Scene::Update`, while `Renderer::RenderScene` (`Renderer.cpp:16-18`) computes its own delta from `rctx_.time_`. They diverge because `rctx_.time_` doubles as the animated-shader timer. Separate the two: carry an `elapsed_time` accumulator for shaders and pass the application's `delta_time` into the render context directly.

- [ ] **`Application` constructor hardcodes scene/layer setup** — `Application.cpp:9-16` directly constructs `TestLayer` and a `Scene` inside the engine's constructor. Scenes and layers should be built outside and passed in, or the engine should expose an `OnInit()` virtual/callback for subclasses.

- [ ] **`key_map` defined in a header** — `Scene.hpp:15-21` defines a `static const std::unordered_map` at namespace scope. Every translation unit that includes it gets its own copy. Move the definition to `Scene.cpp` and add an `extern` declaration in the header, or make it `inline const`.

- [ ] **`Colors.hpp` constants defined in a header** — each `const glm::vec4` gives every including TU its own copy. Use `inline constexpr` (C++17) or move definitions to `Colors.cpp` with `extern` declarations in the header.

---

## Code Quality / Correctness Smells

- [x] **Raw-float `Mesh` constructor hardcodes normals to `{0,0,1}`** — fixed: now computes smooth normals by accumulating face normals at shared vertices and normalizing.

- [ ] **`debug_error` throws from constructors** — if `Mesh::setup` triggers a GL error or `Shader::LoadFile` fails, it throws `std::runtime_error` mid-construction. The partially-constructed object's destructor then runs, which (once the destructor leak is fixed) will call `glDeleteBuffers` on uninitialised or zero handles. Use a factory/`static Create()` pattern or validate before allocating GPU resources.

- [ ] **`Camera.cpp` still has a redundant double include** — `#include "Core/Camera.hpp"` and `#include "Camera.hpp"` both resolve to the same file. Remove the second one.

- [ ] **`Shader::Tag_` is public with a trailing underscore** — the trailing underscore convention is for private members. `Tag_` is public. Either rename to `Tag` (public) or make it private with a `GetTag()` accessor. `Mesh::tag_` has the same issue.

- [ ] **`glUseProgram` called inside `TestLayer` constructor** — `TestLayer.cpp:89-90` binds a shader and sets a uniform during construction, before the render loop starts. This works today but is fragile — if any other shader is bound after construction and before the first draw, the state diverges. Set uniforms in `OnRender` or `OnUpdate` instead.

- [ ] **`Camera::GetViewMatrix()` and `GetProjectionMatrix()` are non-const** — `Camera.hpp:28-29`. Neither method modifies state; both should be `const`. Without it, calling them on a `const Camera` or through a const reference won't compile.

- [ ] **`Model` has no setters for position, rotation, or scale** — `Model.hpp`. All three transform fields are private with no public API to modify them after construction. Every `Model` is permanently stuck at the origin, unrotated, at unit scale.

- [ ] **Shader uniform locations fetched on every call** — `Shader.cpp:48-91`. Every `SetVec4`, `SetMat4`, etc. calls `glGetUniformLocation` each time it is called. This is a driver round-trip per uniform per frame. Cache the results in a `std::unordered_map<std::string, GLint>` populated on first use.