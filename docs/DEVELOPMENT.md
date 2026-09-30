# Development guide

Build, test and architecture reference for Forge.

**Read `SCOPE.md` first.** This engine has a concrete end goal now (a framework capable
of running a networked, online TCG card game for friends) and a Tier 1/Tier 2/Tier 3
definition of "done" — not every gap noted below is in scope for v1 (Tier 3 explicitly
isn't gated on the TCG at all). `ROADMAP.md` tracks the mechanics; `SCOPE.md` tracks the
why and how much.

## Build

The project ships a `Makefile` that wraps CMake:

```bash
make          # configure + build (parallel via nproc)
make run      # build then run from project root
make test     # configure with -DBUILD_TESTS=ON, build, and run EngineCore_tests
make clean    # remove the build/ directory

# Demo app / library-only build (BUILD_DEMO is sticky until toggled off or `make clean`)
make demo     # configure with -DBUILD_DEMO=ON, build OpenGL_App, run it
make lib      # configure with -DBUILD_DEMO=OFF, build only libEngineCore.a — no demo code compiled

# Build type (configure only — CMake caches the value)
make debug
make release

# Feature flags (sticky until toggled off or `make clean`)
make event    # enable LOG_EVENTS
make noevent
make fps      # enable SHOW_FPS
make nofps
```

Or call CMake directly:

```bash
cmake -S . -B build
cmake --build build
# Enable optional flags at configure time:
cmake -S . -B build -DSHOW_FPS=ON -DLOG_EVENTS=ON
```

Run from the **project root** (not `build/`) so the relative shader paths resolve:

```bash
./build/OpenGL_App
```

**Dependencies**: `glfw3`, `glm`, `OpenGL` (glad is bundled in `src/glad/` and `include/glad/`). C++20 required.

## Installing as a library

`EngineCore` is installable and consumable from a separate project via `find_package`:

```bash
cmake -S . -B build
cmake --build build
cmake --install build --prefix ~/.local   # or any prefix; no sudo needed for a user-local one
```

A consuming project then just needs:

```cmake
find_package(Forge REQUIRED)
target_link_libraries(MyTarget PRIVATE Forge::EngineCore)
```

Installs `libEngineCore.a` + every `Forge`/`glad`/`stb_image`/`stb_truetype` header (not
`Demo` — that's this repo's own example app code, not part of the reusable engine) + a
`ForgeConfig.cmake` package under `<prefix>/lib/cmake/Forge/`. **One thing a new consumer
needs to know going in**: a few Forge features load an asset by a path relative to the
*process's working directory* at runtime, not relative to the install prefix —
`DebugOverlayLayer`'s overlay shader, `Text`'s shader pair, `Panel`'s shader pair, the
shared vertex shader, and the default debug font. `cmake --install` copies these specific files to
`<prefix>/share/forge/{shaders,fonts}` as reference copies; a new consumer needs its own
copies (or symlinks) of them at `shaders/...`/`fonts/...` relative to wherever *it* runs
from — same "always run from project root" constraint this repo's own demo already lives
under, just now relevant to a second project too.

## Tests

`make test` is the one-command path (configure + build + run). Manually:

```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build --target EngineCore_tests
./build/EngineCore_tests
```

GTest, discovered per-`TEST()` via `gtest_discover_tests` (so `ctest` and the binary itself report the same individual test names). Test files live in `test/`, one per class, and must be added explicitly to `CMakeLists.txt`'s `EngineCore_tests` sources — there's no glob.

**What's covered**: `Camera` (FOV clamping/sequencing, WASD movement composition including `SetBoost` across both forward and strafe movement, view/projection matrix correctness, orthographic projection, the straight-down `SetUp` degeneracy fix, mouse-look's first-call-primes-only behavior), `Viewport` (aspect-ratio computation across window resize/`SetSize`/`SetViewportPos`, direct rather than only through `Camera`'s projection tests), `Mesh::ParseObjFile` (all four OBJ face-index forms, vertex dedup, n-gon fan-triangulation, degenerate/unknown-line handling, the flat-normal-generation branch), `Mesh::ParseObjFileGroups` (`usemtl`-based grouping in first-seen order, faces before the first `usemtl` landing in the default group, shared vertex pool across groups, per-group flat-normal generation, and that `ParseObjFile` correctly merges multiple groups back into one combined mesh), `Light`/`DirectionalLight`/`PointLight`/`SpotLight` (type identity, direction normalization, `SpotLight`'s degree→cosine cutoff conversion, `ToGPULight()`'s exact packing per light type), `Model::GetModelMatrix()` (translate/rotate/scale composition order), `Vec3Tween` (Linear/EaseOutQuad interpolation, `IsDone`/clamping under `Repeat::None`, `Repeat::Loop`'s `fmod` wraparound, `Repeat::PingPong`'s direction reversal, `SetSpin`'s continuous-angle rotation representing a full turn that plain slerp can't, the finish callback firing once-per-lap rather than once-ever when repeating — all via a `Model(nullptr)` since neither class ever dereferences the mesh pointer), `Prop` (transform-forwarding to every owned part, safe no-op on an empty part list), `Colors::RandomColor()` (bounds/variety invariants), `Forge::Key`/`Forge::MouseButton` (every value checked against GLFW's real `GLFW_KEY_*`/`GLFW_MOUSE_BUTTON_*` constants — the one place in the test suite that includes GLFW, specifically to verify against it), and `ClickableRegion`/`FindTopmostContaining` (rect containment including all four edges, `SetRect` moving/resizing the hit area, click callback invocation/replacement, left-button-only filtering, and `FindTopmostContaining`'s topmost-of-several-overlapping-regions selection — the pure-logic pieces `DragLayer` builds on; `DragLayer` itself isn't tested here for the same reason `Panel`/`Text`/`Button`/`UILayer` aren't — real `Panel`s need a live GL context to even construct).

**What isn't, and why**: anything that touches real GPU resources — `Mesh`'s GPU upload (`setup()`/VAO/VBO/EBO), `Texture`, `Shader`, `ResourceManager::Load*` (all call the above directly), `Engine`/`Scene`/`Renderer` — needs a live GL context, which the test binary doesn't create. `Mesh::ParseObjFile` exists as a separate method specifically because it's the one part of OBJ loading that doesn't need one — `Mesh::Create(filename)` just opens the file and delegates to it. That's the pattern to repeat if more of the GL-dependent classes get pulled apart for testability later: separate the pure-CPU-logic half from the one-time GPU-touching half, and test the former directly.

## Controls

| Key | Action |
|-----|--------|
| ESC | Close application |
| `` ` `` (backtick/grave) | Toggle mouse capture on/off |
| Tab | Cycle to the next registered scene (`Engine::SetScene`) |
| W/A/S/D | Camera movement |
| Space / Left Shift | Camera up / down |
| Mouse | Camera rotation (when captured) |
| Scroll | FOV zoom |
| Q | Randomize the Gold material's color (`LightDemoLayer` demo) |
| Left Click | Click-to-pick: recolor whichever station's material got hit — aims from the window's center pixel while the cursor is captured (Minecraft-style, since GLFW's captured-cursor position isn't a real screen point), or from the actual cursor position in `Normal` mode |
| F3 | Toggle the debug AABB wireframe overlay (`Forge::DebugOverlayLayer`) |

## Architecture

The engine follows an **Engine → Scene → Layer** hierarchy. Forge engine code lives in `Forge` namespace; demo-specific code lives in `Demo` namespace. The two are fully decoupled — core has zero compile-time dependency on demo-layer types.

### Forge layer (namespace `Forge`)

`include/Forge/` and `src/Forge/` are split into four subfolders — purely a navigation aid (the codebase had grown large enough that finding a given file by eye was getting difficult), not a build/dependency boundary: everything still compiles into one `EngineCore` static library, and there's no restriction on what can include what across them. `Core/` — engine bootstrap, GLFW event wiring, resource loading (`Engine`, `EventHandler`, `Event`, `InputEvents`, `Keys`, `Specifications`, `ResourceManager`, `Renderer`). `Scene/` — the `Engine → Scene → Layer` hierarchy itself plus per-frame data (`Scene`, `Layer`, `FrameContext`, `DebugOverlayLayer`). `Rendering/` — geometry, transforms, camera, materials/shaders/textures, picking math (`Mesh`, `Model`, `Camera`, `Material`, `Shader`, `Texture`, `Prop`, `RayCast.hpp`, `Colors`, `Tweens`). `Lighting/` — `Light`, `Lights`. `Demo/` stays flat (small enough not to need this). Every `#include` uses the fully-qualified path (e.g. `#include "Forge/Rendering/Mesh.hpp"`) — the codebase used to also have a handful of stray *unqualified* `#include "Mesh.hpp"`-style includes (some genuinely functional, resolving only because `include/Forge` was flatly on the include path; others harmless leftover duplicates of an already-qualified include on the line above) — both were cleaned up as part of this move, the functional ones fixed to the qualified form, the dead duplicates deleted outright.

- **`Engine`** — owns the GLFW window, `Renderer`, `EventHandler`, `ResourceManager`, and a list of `Scene`s. Drives the main loop: `Update` → `RenderScene` → `SwapBuffers` → `PollEvents`. `RaiseEvent(Event&)` is the entry point every `EventHandler` callback funnels through: it first runs fixed, always-on bookkeeping that isn't app policy (`WindowResize` updates `FrameContext`'s window size + `glViewport`; `WindowLostFocus` clears the held-key table, see below — neither is gated behind what the app handler decides), then calls the app-settable global handler via `HandleEvent(Event&)` (set with `SetEventHandler(std::function<bool(Event&)>)`) before forwarding to the active scene — same `false` = consumed/stop, `true` = pass-through convention as `Layer::OnEvent`. No app policy is hardcoded in `Engine` anymore: `main.cpp`'s registered handler is what implements ESC-closes-the-window/Tab-cycles-scenes/backtick-toggles-cursor-capture today, via the public methods `CloseWindow()`, `NextScene()`/`PrevScene()`, and `SetCursorMode(CursorMode)`/`GetCursorMode()` — a different app could bind those differently, or not at all (e.g. ESC opening a pause menu instead of closing). Scenes are created outside `Engine` (in `main.cpp`) and added via `AddScene(shared_ptr<Scene>)`, which calls `scene->OnLoad(rmanager, rctx)` to initialize the scene and returns its index. `SetScene(index)` switches which registered scene is active — bounds-checked, silently ignores an out-of-range index; applied via a `next_scene_` member at the top of `Run()`'s loop (not immediately) so `current_scene_` can never change mid-frame between `Update()` and `RenderScene()`. `Run()` calls the newly-active scene's `ResetMouse()` right when the switch is applied, since a scene's camera may have stale mouse-tracking state from whenever it was last active. A scene switch also runs the Suspend/Resume lifecycle around that: `Suspend()` fires on the outgoing scene, then `Resume()` on the incoming one (re-syncs every camera's stored window size — resize events only reach the active scene — then calls the `OnResume` hook), then `ResetMouse()`. **Deliberate lifecycle quirk (accepted 2026-07-13):** the *first* scene never receives `Resume()`/`OnResume()` at startup, because `AddScene` marks the first registered scene active immediately (`current_scene_ = next_scene_ = 0`), so `Run()`'s switch branch is skipped on frame one — that immediate marking is also what fixed a real `scenes_[-1]` startup segfault (`Suspend()` used to run while `current_scene_` was still the `-1` sentinel). Initial-activation work therefore belongs in `OnSceneBoot()`. Renaming `OnResume` to something like `OnSceneSwitch` was considered and deferred as unnecessary; revisit this (or restore a `-1` sentinel with a guarded `Suspend`) only if a scene ever needs identical work on *every* activation, boot included.

- **Held-key queries** — `Engine::IsPressed(Key)`/`IsRepeat(Key)` read a `std::array<KeyState, 348>` (`KeyState::Released`/`Pressed`/`Repeat`, `Released = 0` so the zero-initialized array starts every key as not-held; `IsPressed` treats `Repeat` as still-down too) kept live by `RaiseEvent` on every real `KeyPressedEvent`/`KeyReleasedEvent`. Exists for input a single edge-triggered event can't express on its own — e.g. checking a modifier is currently held from inside a different key's `KeyPressed` handler, for something like an Alt+G shortcut. Cleared entirely (`fill(KeyState::Released)`) on `WindowLostFocusEvent`, since GLFW never sends a `KeyReleased` for a key let go while the window wasn't focused — left unhandled, that key would read as permanently held. `Key::Unknown` is explicitly skipped on both press and release, in both the table-update code and by choice not tracked at all: casting it (`-1`) to an array index would wrap to a huge unsigned value and write out of bounds.

- **`CursorMode`** (`include/Forge/Core/InputEvents.hpp`) — `Captured`/`Normal`, hand-transcribed to GLFW's real `GLFW_CURSOR_DISABLED`/`GLFW_CURSOR_NORMAL` integer values (212995/212993) the same way `Keys.hpp`'s `Key`/`MouseButton` are — deliberately no `#include <GLFW/glfw3.h>` needed for it, keeping the same GLFW-free invariant `Keys.hpp` documents (and `test_keys.cpp` guards) for the rest of Forge's input enums.

- **`WindowSpecification` / `ApplicationSpecification`** (`include/Forge/Core/Specifications.hpp`) — POD structs for window config: title, width/height (default 1080×1080), `isResizable`, `VSync`. `Engine::Init` branches on `VSync` to call `glfwSwapInterval(1 or 0)` after `glfwMakeContextCurrent`.

- **`ResourceManager`** (`include/Forge/Core/ResourceManager.hpp`) — shared between `Engine`, `Scene`, and `Layer`. Deduplicates GPU asset loads via `weak_ptr` caching: if the shared_ptr is still alive it returns the existing object; otherwise reloads from disk.
  - `LoadMesh(filename)` — loads an OBJ from `models/`, caches by filename.
  - `LoadShader(vertPath, fragPath, tag)` — caches by `vertPath + "||" + fragPath`.
  - `LoadTexture(filename)` — loads an image via stb_image, uploads to GPU, caches by filename.
  - `LoadMaterial(shader, tag, ambient = ..., diffuse = ..., specular = ..., shininess = ...)` — caches by tag; delegates to `Material::Create(shader, tag, ambient, diffuse, specular, shininess)`. Default ambient/diffuse/specular/shininess values are an average across the existing Blinn-Phong presets.
  - `Engine` constructs one instance, passes it into each `Scene` via `OnLoad`; scenes pass it to `Layer` constructors.

- **`Scene`** (abstract) — holds a list of `Layer`s, a list of `Camera`s (each owning its own `Viewport`, see below), and the lights UBO. Pure virtual interface:
  - `OnSceneBoot()` — called by `OnLoad` after context is stored; create cameras, add layers, add lights here.
  - `OnUpdate(float delta_time)` — per-frame scene logic (e.g. camera update).
  - `OnEvent(Event&)` — handle input and forward to layers.
  - `OnMouseCapture()` — called when cursor capture is toggled on; reset mouse tracking here.
  - `OnResume(shared_ptr<FrameContext>)` — scene-switch hook, called (via the non-virtual `Resume()`) whenever the scene becomes active through `Engine::SetScene`; **not** called for the first scene's initial activation at startup — see the `Engine` bullet's lifecycle note. `Suspend()` is its virtual-but-not-pure counterpart (default no-op) and fires on the outgoing scene.

  Final methods (non-overridable): `Update`, `Render`, `AddLayer`, `Destroy`, `SetBackgroundColor`/`SetSkyboxBackground`/`SetSkydomeBackground`, `DrawBackground`, `ApplyViewport`, `ResetMouse`, `Resume`, `GetLayerByName(string)` (linear scan over `layers_` by `Layer::GetName()`, same by-name lookup shape as `materials_`'s existing by-tag lookup; every demo scene's `F3` handler uses this to find and toggle its `DebugOverlayLayer`). `Update` calls `OnUpdate` then each layer's `OnUpdate`. `Render()` loops over *every* camera in `cameras_` each frame — `ApplyViewport(i)`, `UpdateFrameContext(i)`, a scissored depth clear, `DrawBackground()`, then every layer's `Render()` — wrapped in `glEnable`/`glDisable(GL_SCISSOR_TEST)` so one camera's clear can't bleed into another's viewport; `active_camera_` is purely an input-routing index and plays no role in which cameras render. `Background_Type` (`Solid`/`Skybox`/`Skydome`/`None`) is set via the three `Set*Background` calls — `Skybox`/`Skydome` are both fully implemented (procedural, no textures) and lazily create their shader/mesh on first draw, cached as `Scene` members; `Scene` has a real destructor to release the skydome's raw VAO. `ResetMouse()` resets the active camera's mouse tracking — call on a scene that's about to become newly active (see `Engine::SetScene`).

- **`Layer`** (abstract base) — the extension point for application logic. Constructor takes a `std::string name` (every subclass must pass one through). Subclasses implement:
  - `OnEvent(Event&, shared_ptr<FrameContext>) → bool` — return `false` to consume the event and stop propagation; return `true` to pass to the next layer. Gained the `FrameContext` parameter alongside the click-to-pick work (see `Camera`/`RayCast.hpp` below) — every `Scene::OnEvent` forwards its own `fctx_` when dispatching to layers. `OnUpdate()` (next) still doesn't get one — a known, separately-tracked gap (`ROADMAP.md`/`SCOPE.md` Tier 2).
  - `OnUpdate()` — per-frame logic.
  - `OnRender(shared_ptr<FrameContext>)` — a hook for anything layer-specific that needs to happen before the draw loop below; can be a no-op (it is, in `LightDemoLayer`).
  - `Transition()` / `Suspend()` / `Destroy()` — have default empty implementations; only override if needed.

  `Render(shared_ptr<FrameContext>)` (non-virtual, final) is the actual draw dispatch: calls `OnRender` first, then binds each Material once and sets `uModel`/`uNormalMatrix` per model via `material->GetShader()->SetMat4(...)`/`SetMat3(...)` before calling `model->Draw()`. Every shader bound by a Material is expected to declare uniforms under these exact names — this is Forge's uniform-naming contract, not a per-shader option. `Render()` is a no-op while `IsShown()` is false (see `Show()`/`Hide()`/`SetShow()` below). Each `Layer` stores two members backing draw dispatch: `materialModels_` (`std::unordered_map<shared_ptr<Material>, vector<shared_ptr<Model>>>`) is the actual authority `Render()` iterates over — the map key is a `shared_ptr`, so the same Material object must be used consistently. `materials_` (`vector<shared_ptr<Material>>`) is a parallel list used for tag-based lookup (`std::ranges::find_if` by `material->GetTag()`). Keep both in sync when adding materials.

  Also on `Layer`: `GetName()`; `IsShown()`/`Show()`/`Hide()`/`SetShow(bool)` (backs the F3 debug-overlay toggle, see `DebugOverlayLayer` below); `GetAllModels()` (flattens `materialModels_` into a plain list, for something outside the `Layer` — e.g. `DebugOverlayLayer` — that needs to see what it's drawing without reaching into the private buckets above); and two click-to-pick helpers built on `RayCast.hpp` (below) — protected `GetClickedObj(Ray) const` (walks `materialModels_`, returns the closest hit's `(Material, Model)` as `std::optional<std::pair<...>>`, or `nullopt`; brute-force over every model in the layer, which is fine — ray/AABB is cheap and this only runs on a click, not every frame, at TCG-scale object counts) and `GetClickedOnObj(MouseButtonPressedEvent&, shared_ptr<FrameContext>)` (turns a real mouse click into a ray — see `Camera::ScreenPointToRay`'s note on `Captured` vs `Normal` cursor mode below — then calls `GetClickedObj`). `Scene::GetLayerByName(string)` does the equivalent by-name lookup one level up, over a `Scene`'s own `layers_`.

- **`Renderer`** — owns a `shared_ptr<FrameContext>` internally, via `GetFrameContext()`. Each frame: sets `delta_time_` and `time_`, calls `scene->DrawBackground()`, then `scene->Render()`.

- **`FrameContext`** (`include/Forge/Scene/FrameContext.hpp`, formerly `RenderContext`) — struct passed to `OnRender` as a `shared_ptr`; contains `view_`, `projection_`, `camera_position_`, `aspect_ratio_`, `delta_time_`, and `time_` (accumulated elapsed time for animated shaders). `Scene::UpdateFrameContext()` refreshes it once per frame. Also carries `cursor_mode_`, mirroring `Engine`'s own (kept in sync by `Engine::SetCursorMode`) — since `FrameContext` (not `Engine`) is what reaches `Layer::OnEvent` now, this is how `Layer::GetClickedOnObj` can tell whether the cursor is a free on-screen point (`Normal`) or a captured/FPS-style one (`Captured`) without a separate channel back to `Engine`.

- **`Camera`** — perspective camera with WASD + mouse-look. Boolean movement flags set by key events; `Update(delta_time)` integrates them each frame at `speed_ + boost_` (see `SetBoost`, wired to Left Ctrl in `LightDemoScene::OnEvent` as a sprint modifier — 0 by default). `Zoom(yOffset)` adjusts FOV. Owns a `Viewport` (normalized `[0,1]` `x,y,width,height` fraction of the window, plus the window's current pixel size — `SetWindowSize`/`Camera::UpdateViewportSize` only ever update the *window* size a viewport is measured against, never its own fractions, so a viewport survives a resize correctly on its own) whose `Apply()` sets both `glViewport` and `glScissor` from it (`Camera::ApplyViewport()`/`Scene::ApplyViewport(i)` forward to it) and whose `GetAspectRatio()` feeds `GetProjectionMatrix()`. `Viewport::GetPixelRect()` exposes that same x/y/width/height-in-pixels math without touching GL state (`Apply()` now calls it too, instead of duplicating the math). `SetPosition`/`SetYawPitch` directly set pose, bypassing `CameraMove`/`Update`/`Rotate` — for a fixed/scripted camera (e.g. a picture-in-picture overview) that never receives input.

  `ScreenPointToRay(screenX, screenY, Ray&) const` — builds a world-space `Forge::Ray` (see `RayCast.hpp` below) from a screen-space pixel, for click-to-pick; a thin wrapper pulling this camera's own `Viewport::GetPixelRect()` into the shared `UnprojectScreenPoint()` free function. `Layer`-level code (`GetClickedOnObj`, above) can't call this directly — a `Layer` only ever has a `FrameContext`, not a `Camera` — so it calls `UnprojectScreenPoint()` itself with `(0, 0, ctx->window_width_, ctx->window_height_)` as an approximation of "the active camera's rect"; exact for every demo scene today (the camera that receives input always fills the whole window), only an approximation the moment input ever routes to a non-full-window camera (e.g. a split-screen inset).

- **`RayCast.hpp`** (`include/Forge/Rendering/RayCast.hpp`, header-only, same shape as `Prop.hpp`) — the click-to-pick math, usable independent of `Camera`/`Model`/`Layer`:
  - `Ray` (`origin`, `direction`) and `AABB` (`min`, `max`) — each its own small type not because either needs to be, but because both cross a producer/consumer boundary (built in one place, tested in another) where two loose `vec3`s could get silently swapped or passed in the wrong order.
  - `IntersectRayAABB(Ray, AABB, float& outT)` — the slab method: per axis, the range of `t` where the ray is between that axis's two bounding planes, intersected across all three axes; `tMin` clamped to `0` so a box behind the ray's origin is never a hit.
  - `IntersectRayPlane(Ray, planePoint, planeNormal, float& outT)` — solves for `t` directly. Built for a future click-and-drag-along-a-plane feature (dragging a card across a table); no caller yet.
  - `UnprojectScreenPoint(screenX, screenY, viewportX/Y/Width/Height, view, projection, Ray& outRay)` — screen pixel → NDC → `inverse(projection * view)` at both the near (`z=-1`) and far (`z=1`) plane in NDC, producing two world-space points; `w` is divided out by hand, since the GPU's own perspective divide (automatic going forward) doesn't happen going backward through the inverse matrix. Returns false if the point falls outside the given viewport rect.

- **`Mesh`** — owns GPU resources (VAO/VBO/EBO). Non-copyable, movable. Constructors are private — use the factory:
  - `Mesh::Create(tag, vertices, indices, drawMode)` — from `Vertex` structs (position + normal + texCoords)
  - `Mesh::Create(tag, vertices, indices, drawMode)` — from raw `GLfloat` arrays (position only; normals computed via face accumulation)
  - `Mesh::Create(filename)` — loads an OBJ file; used by `ResourceManager::LoadMesh`. If the OBJ has no `vn` data, flat normals are generated by expanding to one vertex per index (no sharing), so each triangle gets a clean consistent normal.
  - `Mesh::CreateGroups(filename)` — loads an OBJ file that uses `usemtl` to mark multiple material groups, returning one `MeshGroup` (`materialName` + `shared_ptr<Mesh>`) per distinct name instead of a single `Mesh` for the whole file. Built on `Mesh::ParseObjFileGroups` (the CPU-only counterpart to `ParseObjFile`, same unit-testable-without-a-GL-context rationale) — v/vn/vt parsing/indexing stays shared across the whole file as real OBJ semantics require; only face-to-material association is grouped. A referenced `mtllib` is never opened — `usemtl` names are just lookup keys the caller resolves into real `Material`s itself (e.g. via `ResourceManager::LoadMaterial`). `Mesh::ParseObjFile`'s own long-standing single-mesh contract is unaffected: it's defined in terms of `ParseObjFileGroups` and merges multiple groups back into one combined buffer if a caller uses the old single-mesh path directly on a multi-material file.

  `Mesh` also computes and caches its own local-space (untransformed) AABB — `boundsMin`/`boundsMax`, updated once by `ComputeBounds()` from both GPU-uploading constructors, exposed via `GetLowerBounds()`/`GetUpperBounds()`. Exists for `Model::GetWorldBounds()` (below) to build on for click-to-pick — see `RayCast.hpp`.

- **`Model`** — wraps a `shared_ptr<Mesh>` with a transform (position, scale, quaternion rotation) and an optional `shared_ptr<Material>`. `GetModelMatrix()` returns the composed mat4. Public setters: `SetPosition(vec3)`, `SetScale(vec3)`, `SetScale(float)`, `SetRotation(quat)`, `SetRotation(angle_radians, axis)`, `SetMaterial(shared_ptr<Material>)`. Note: the `Layer::materialModels_` map is the authority for rendering — the model's material reference is a convenience for queries only (and, as of the click-to-pick work, is never actually set on any of `LightDemoLayer`'s models — `GetMaterial()` on any of them returns `nullptr`; the actual material is only ever known via `materialModels_`, which is exactly why `Layer::GetClickedObj()` returns the material it resolved rather than making a caller ask the hit `Model` for one).

  `GetWorldBounds()` (`include/Forge/Rendering/RayCast.hpp`'s `Forge::AABB`, min+max) — pushes `Mesh::GetLowerBounds()`/`GetUpperBounds()`'s local-space box through **all 8 corners** of `GetModelMatrix()`, then re-min/maxes them; transforming only the 2 corner points isn't enough once rotation is involved. Built for click-to-pick (`Layer::GetClickedObj()`); deliberately produces an axis-aligned box, not an oriented one — a rotated model's box visibly grows/shrinks as it turns rather than turning with it, accepted as fine for now (see `ROADMAP.md`'s Open Architecture entry for the full reasoning).

- **`Prop`** (`include/Forge/Rendering/Prop.hpp`, header-only) — groups several `Model`s that represent one logical multi-part object (e.g. the `usemtl` groups `Mesh::CreateGroups` produces) so they move as a unit: owns `vector<shared_ptr<Model>>` and forwards `SetPosition`/`SetScale`/`SetRotation` to every part. Deliberately not a rendering concept — every part still has to be registered into a `Layer`'s `materialModels_`/`materials_` individually for it to actually render; `Prop` only keeps their transforms in sync.

- **`DebugOverlayLayer`** (`include/Forge/Scene/DebugOverlayLayer.hpp`/`src/Forge/Scene/DebugOverlayLayer.cpp`) — a real engine-level `Layer` subclass, not demo-only: given a list of other `Layer`s to watch, it snapshots every `Model` across them (via `Layer::GetAllModels()`) once at construction and draws a live wireframe box around each one's `Model::GetWorldBounds()` every frame — a visual debug aid for exactly the AABB click-to-pick relies on, built because there's no text/UI rendering yet to build a numeric HUD with (`SCOPE.md` Tier 2). One shared unit wireframe cube `Mesh` (`GL_LINES`, corners at `±0.5`) is built once; `OnUpdate()` re-fits each tracked box's `SetPosition`/`SetScale` to its source model's current world AABB every frame (no rotation applied — see `Model::GetWorldBounds()`'s AABB-not-OBB note). Uses a new minimal unlit fragment shader, `shaders/debug_overlay_fragment.glsl` (pairs with the existing shared `vertex.glsl`) — just outputs `uBaseColor`; doesn't need to declare `uMaterial`/`uHasTexture`/etc. at all, since `Shader::SetX` on a uniform name a shader doesn't declare just resolves to location `-1`, a defined no-op. Toggle with the inherited `Show()`/`Hide()`/`SetShow()` — every demo scene constructs one named `"Overlay"`, hidden by default, and binds `F3` (`Scene::GetLayerByName("Overlay")` + `SetShow(!IsShown())`) to it in `OnEvent`.

- **`Font`**/**`Text`** (`include/Forge/Rendering/Font.hpp`/`Text.hpp`) — `Font::Create(ttfPath, pixelHeight)` bakes a `.ttf` into one GPU glyph atlas via stb_truetype (printable ASCII only, 32..127 — no Unicode). `Text::Create(rmanager, font, string, x, y, color)` builds a drawable screen-space string against it; `Draw(windowWidth, windowHeight)` bypasses the normal `Material`/`materialModels_` dispatch entirely and drives its own shader (`shaders/text_vertex.glsl`/`text_fragment.glsl`) and a fixed pixel-space orthographic projection directly — the same reasoning `Panel` below follows for the same reason (no hook in `Material::Bind()` for "ignore the scene camera, use screen pixels instead"). `SetString`/`SetPosition` rebuild the glyph-quad mesh; `SetColor` doesn't.

- **`ClickableRegion`**/**`Button`**/**`UILayer`** (`include/Forge/Rendering/ClickableRegion.hpp`/`Button.hpp`, `include/Forge/Scene/UILayer.hpp`) — `ClickableRegion` is a screen-space rect + click callback with no visual of its own (pixels, origin top-left, y down — same convention as `MouseButtonEvent`/`Text`). `Button` pairs one with a `Text` label (padded hit-region around the label's own bounding box). `UILayer` is the `Layer` that actually owns and draws a scene's `Panel`s/`Text` labels/`Button`s every frame and forwards left-clicks to each `Button`'s hit-test — guarded against `CursorMode::Captured` (a captured cursor has no real on-screen position to hit-test against). `FindTopmostContaining(vector<ClickableRegion>, x, y)` is a free function picking whichever of several overlapping regions is "on top" (last in the vector = topmost, no separate z-index) — used by `DragLayer` below for both pick-up and drop-zone resolution.

- **`Panel`** (`include/Forge/Rendering/Panel.hpp`) — a drawable, screen-space colored rectangle with an optional border, Forge's generic screen-space quad primitive (there wasn't one before this — `Button` used to just be a label with an invisible click target, no drawable background). Same `Text`-style bypass-`Material` direct-shader-and-GL-state `Draw()`, its own shader pair (`shaders/panel_vertex.glsl`/`panel_fragment.glsl`) — the border is drawn by the fragment shader from a single quad (`borderThicknessPx = 0`, the default, means no border), not separate geometry. `SetPosition`/`SetSize`/`SetRect` rebuild the (4-vertex) quad; color/border changes don't.

- **`DragLayer`** (`include/Forge/Scene/DragLayer.hpp`, header-only) — generic screen-space drag-and-drop: a `Layer` that owns a `Panel` per registered draggable item plus a set of named drop-zone rects, and moves/redraws the dragged item's `Panel` to follow the cursor between a left-press and left-release, same `CursorMode::Captured` guard as `UILayer`. `OnDrop(id, zoneId)` (zoneId empty if released over no zone) decides accept/reject — accepted means the `Panel` stays wherever it now is (that becomes its new home); rejected (or no callback at all) snaps it back to its position before the drag started. `OnDragMove(id, x, y)` fires on every move (and once more on snap-back) for a caller that wants something else (e.g. a name label) to ride along with the dragged `Panel`. Deliberately knows nothing about what's being dragged or dropped — same game-agnostic spirit as `ClickableRegion`/`Button`.

- **`Shader`** — loads vertex + fragment GLSL from separate file paths, compiles/links, exposes typed uniform setters. Constructor is private — use the factory: `Shader::Create(vertexPath, fragmentPath, tag)`. Each shader has a `Tag_` string for lookup. Uniform locations are cached on first use.

- **`Texture`** (`include/Forge/Rendering/Texture.hpp`, `src/Forge/Rendering/Texture.cpp`) — owns a GPU texture. Constructor is private — use the factory: `Texture::Create(path)`. Loads via stb_image with vertical flip. `Bind(slot)` calls `glActiveTexture(GL_TEXTURE0 + slot)` then `glBindTexture`. `Unbind()` unbinds. Destructor calls `glDeleteTextures`.

- **`Material`** (`include/Forge/Rendering/Material.hpp`, `src/Forge/Rendering/Material.cpp`) — owns a `shared_ptr<Shader>`, an optional `shared_ptr<Texture>`, and Blinn-Phong surface parameters. Constructor is private — use the factory: `Material::Create(shader, tag)`. Key method: `Bind(shared_ptr<FrameContext>)` calls `shader->Use()`, uploads all camera/frame uniforms from ctx, uploads material uniforms (`uMaterial.*`, `uBaseColor`), and binds texture if present. `GetShader()` exposes the shader for per-model uniform calls (e.g. `uModel`). Setters: `SetColor`, `SetAmbient`, `SetDiffuse`, `SetSpecular`, `SetShininess`, `SetTexture`. Built-in presets (static factory methods, classic OpenGL Blinn-Phong values): `Gold`, `Silver`, `Bronze`, `Chrome`, `Copper`, `Emerald`, `Ruby`, `Pearl`, `Obsidian`, `Light`.

- **`Light`** (`include/Forge/Lighting/Light.hpp`) — abstract base class with `color_` (vec3) and `intensity_` (float). Uses `LIGHT_CLASS_TYPE(name)` macro (mirrors `EVENT_CLASS_TYPE`) to wire `GetLightType()`, `GetStaticType()`, `GetName()`. `LightType` enum: `Directional=0`, `Point`, `Spot`. Concrete subclasses in `include/Forge/Lighting/Lights.hpp` (header-only, no `.cpp`):
  - `DirectionalLight` — adds `direction_` (vec3, normalised on construction/set)
  - `PointLight` — adds `position_` (vec3), attenuation floats `constant_`, `linear_`, `quadratic_` (defaults: 1.0, 0.09, 0.032 ≈ 50-unit range)
  - `SpotLight` — adds `position_`, `direction_`, `innerCutoff_`, `outerCutoff_` (stored as cosines; constructor takes degrees)

- **`EventHandler`** — registers GLFW callbacks and fires typed `Event` subclasses through the `WindowSpecification::EventCallback`. The one place in Forge allowed to know GLFW's key/button numbering — translates a raw GLFW key/button code into `Forge::Key`/`Forge::MouseButton` via a plain `static_cast` (see `Keys.hpp` below) before constructing any `KeyEvent`/`MouseButtonEvent`. `glfwSetWindowFocusCallback` only raises a `WindowLostFocusEvent` on the losing transition (`focused == GLFW_FALSE`) — regaining focus raises nothing, since nothing needs to react to that side. The mouse-button callback also calls `glfwGetCursorPos` itself (GLFW's callback doesn't supply a position) to populate `MouseButtonPressedEvent`/`MouseButtonReleasedEvent`'s `GetX()`/`GetY()`.

- **`Keys.hpp`** (`include/Forge/Core/Keys.hpp`) — `Forge::Key`/`Forge::MouseButton`, engine-owned enums with no GLFW dependency. Their values are deliberately hand-wired to match GLFW's own `GLFW_KEY_*`/`GLFW_MOUSE_BUTTON_*` numbering (not derived from `<GLFW/glfw3.h>` — this header includes nothing from GLFW), which is what lets `EventHandler` translate with a `static_cast` instead of a lookup table. `test/test_keys.cpp` guards every value against GLFW's real constants. Not every GLFW key has an entry (e.g. numpad, print screen) — add one with its matching GLFW value if a future feature needs it.

### Event system

Events inherit from `Forge::Event`. The `EVENT_CLASS_TYPE(TypeName)` macro wires up `GetEventType()` / `GetName()` / `GetStaticType()`. Concrete types live in `include/Forge/Core/InputEvents.hpp`: `KeyPressedEvent`/`KeyReleasedEvent`, `MouseButtonPressedEvent`/`MouseButtonReleasedEvent`, `MouseMovedEvent`, `MouseScrolledEvent`, `WindowCloseEvent`, `WindowResizeEvent`, and `WindowLostFocusEvent` (no payload — raised only on the focus-lost transition, see `EventHandler` above; `Engine::RaiseEvent` clears the held-key table on it). `KeyEvent::GetKeyCode()` returns a `Forge::Key`, and `MouseButtonEvent::GetMouseButton()` returns a `Forge::MouseButton` — neither exposes a GLFW type or code. `MouseButtonEvent` (and both subclasses) also carry `GetX()`/`GetY()` — the cursor's screen-space position (pixels, origin top-left, y down — same convention as `MouseMovedEvent`) at the moment the button event was raised, added for click-to-pick since GLFW's mouse-button callback doesn't hand you a position on its own (see `EventHandler` below).

`Layer::OnEvent` returns `bool`: **`false` = event consumed, stop propagation; `true` = pass to next layer**. `Scene::OnEvent` iterates layers in reverse order and breaks on the first `false` return.

### Demo layer (namespace `Demo`)

Demo-specific code lives in `src/Demo/` and `include/Demo/`. All types are in the `Demo` namespace. Built only when `BUILD_DEMO=ON` (the default — see Build above); `-DBUILD_DEMO=OFF`/`make lib` compiles none of it.

`LightDemoLayer` (`include/Demo/LightDemoLayer.hpp`, `src/Demo/LightDemoLayer.cpp`) — concrete `Forge::Layer` subclass shared by both scenes below. Builds a floor (hardcoded `Vertex` quad, no OBJ — same "raw vertex array" pattern `Scene::DrawSkyboxBackground` uses for its cube) plus six "stations" laid out along it: one cube per Blinn-Phong preset (Gold/Silver/Ruby/Emerald), each paired with a dedicated light positioned/aimed at it in whichever scene's `OnSceneBoot`, a fifth station for the textured crate (diffuse+specular mapping), and a sixth for a sub-meshed signpost (`models/signpost.obj`, a post + board as two `usemtl` groups loaded via `Mesh::CreateGroups` into a `Forge::Prop` so both parts move as one object). `Q` (not `Tab` — `Tab` is reserved by `Engine` for scene switching) randomizes the Gold material's color via `material->SetColor()`.

`LightDemoScene` (`include/Demo/LightDemoScene.hpp`, `src/Demo/LightDemoScene.cpp`) — concrete `Forge::Scene` subclass, one free-fly camera over a `LightDemoLayer` floor, `Skybox` background. `OnSceneBoot` adds one light per station: three point lights (warm key, cool rim, warm rim), one spot light aimed straight down, and one directional "sun" lighting every station uniformly (the skybox's own sun disc is aimed opposite this direction, so the two agree). Camera movement in `OnEvent`, camera update in `OnUpdate`.

`MultiCameraDemoScene` (`include/Demo/MultiCameraDemoScene.hpp`, `src/Demo/MultiCameraDemoScene.cpp`) — concrete `Forge::Scene` subclass exercising split-screen: a full-window free-fly main camera plus a fixed picture-in-picture camera (`Camera::SetPosition`/`SetYawPitch`, an elevated overview looking down over the whole station layout — genuinely different from the main camera, not a second copy of the same pose), both rendering the same `LightDemoLayer` content and light layout as `LightDemoScene`, but with a `Skydome` background instead of `Skybox` (so both techniques get exercised, and the two very different camera angles double as a check that the sky looks right from both). Same `OnEvent`/`OnUpdate` shape as `LightDemoScene`, but its `WindowResize` handling loops over every camera, not just the active one.

`Colors.hpp` (`include/Forge/Rendering/Colors.hpp`) provides named `glm::vec4` constants in `Forge::Color_A1` (Red, Green, Blue, White, Black, Gray, Yellow, Cyan, Magenta, Orange, Purple, Pink, Lime, SkyBlue, Brown) and `RandomColor()`.

### Assets

OBJ models live in `models/`. `ResourceManager::LoadMesh` resolves paths relative to CWD, so always run the binary from the project root.

### Shaders

Shaders are GLSL files in `shaders/`. Each `Shader` is constructed with a pair of paths: `Shader::Create(vertexPath, fragmentPath, tag)`. **Always run the binary from the project root** — paths are relative to CWD.

- `vertex.glsl` — shared vertex shader. Inputs: `aPos` (loc 0), `aNormal` (loc 1), `aTexCoords` (loc 2). Outputs: `vNormal` (world-space, normal-matrix transformed), `vFragPos` (world-space position), `vTexCoords`. Uniforms: `uModel`, `uView`, `uProjection`.
- `fragment.glsl` — Blinn-Phong fragment shader, currently active. Receives `vNormal`, `vFragPos`, `vTexCoords`. Reads `uMaterial` struct, `uBaseColor`, `uHasTexture`/`uTexture`, `uHasSpecularMap`/`uSpecularMap`, `uCameraPos`, `uTime`, and loops over the real `layout(std140, binding=0)` lights UBO (directional/point/spot, see Step 3 in `ROADMAP.md`) — no hardcoded light.
- `solid_color.glsl` — legacy unlit shader; reads `uniform vec4 triangle_color`. No longer used by `LightDemoLayer` but kept for reference.
- `time_color.glsl` — animated unlit shader; reads `uniform float uTime`.
- `skybox_vertex.glsl`/`skybox_fragment.glsl` — `Scene::DrawSkyboxBackground`'s shaders. Vertex shader strips the view matrix's translation (`mat3(uView)`, so the cube always surrounds the camera) and pins depth to the far plane (`gl_Position = vec4(pos.xy, pos.w, pos.w)`). Fragment shader: zenith/horizon/ground gradient plus a sun disc, no textures.
- `skydome_vertex.glsl`/`skydome_fragment.glsl` — `Scene::DrawSkydomeBackground`'s shaders. Vertex shader takes no vertex attributes at all — builds a full-screen triangle purely from `gl_VertexID`. Fragment shader reconstructs the view ray per-pixel via `uInvViewProj` (`inverse(projection * view)`), a different (screen-space) technique from the skybox's, independently-tuned gradient + horizon haze.

Standard vertex uniforms: `uModel`, `uView`, `uProjection`.
`Material::Bind` sets: `uView`, `uProjection`, `uCameraPos`, `uTime`, `uMaterial.ambient`, `uMaterial.diffuse`, `uMaterial.specular`, `uMaterial.shininess`, `uBaseColor`, `uHasTexture`, `uTexture`.

## Utility macros (`include/Utils.hpp`)

- `debug_info(x)` — yellow console output (no-op in release)
- `debug_warn(x)` — magenta console output
- `debug_error(x)` — red console output + `throw std::runtime_error`
- `n_rgb(x)` — converts 0–255 integer to 0.0–1.0 float

FPS logging in `Engine::Run` is guarded by `#ifdef SHOW_FPS` — use `make fps` or `-DSHOW_FPS=ON` at configure time. Event logging is guarded by `#ifdef LOG_EVENTS` — use `make event` or `-DLOG_EVENTS=ON`. Both are off by default.

## Known issues / next steps

`ROADMAP.md` tracks open bugs/architecture items; `SCOPE.md` tracks what's actually in
scope for v1 (short version: Tier 1, the bug/architecture backlog, is closed; Tier 3 —
multi-camera/split-screen, Skybox/Skydome, sub-mesh support — is fully done. Tier 2:
2D/orthographic camera mode + the picking primitives (`RayCast.hpp`, `Model::GetWorldBounds()`,
`Camera::ScreenPointToRay()`, `Layer::GetClickedObj()`/`GetClickedOnObj()`) are done;
`FrameContext` reaches both `Layer::OnEvent` and `Layer::OnUpdate` now. Text/UI rendering
(`Font`/`Text`, `ClickableRegion`/`Button`/`UILayer`) and a generic screen-space quad/panel
primitive with drag-and-drop (`Panel`, `Forge::DragLayer` — see their own bullets below) are
also done. What's left for Tier 2 is client-server networking only.

The lighting pipeline (UBO, real `Light` objects, end-to-end texture loading including a
second specular slot) is fully done — see `ROADMAP.md`'s Steps 6–7.

Still real:
- **`stb_image` in include path** — `include/stb_image/stb_image.h` is the single-header library. `#define STB_IMAGE_IMPLEMENTATION` is in `src/Forge/Rendering/Texture.cpp` only — do not add it anywhere else.
- **OBJ flat normal generation** — when an OBJ has no `vn` data, `Mesh::Create(filename)` expands to one vertex per index (no sharing) to give each triangle a clean face normal. This is correct for flat shading but means more vertices than the original OBJ.
