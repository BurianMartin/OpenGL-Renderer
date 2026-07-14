# Forge Roadmap

**Forge** is this project's name — short for **F**lexible **O**penGL **R**eal-time
**G**raphics **E**ngine, a C++20 OpenGL engine (`Engine → Scene → Layer`)
being built toward a networked TCG card game for friends, not a general-purpose public
library. See `README.md` for the full pitch and current status.

Tracks open bugs, design decisions, and implementation plans. **See `SCOPE.md` first**
for what this engine is actually for and how much of this file's backlog is in scope —
Tier 3 (see `SCOPE.md`) is a small set of engine-capability items not gated on the TCG at
all, and Redline's phase order now follows `SCOPE.md`'s priority rather than the order it
was originally written in.

---

## ✅ Step 6 & Step 7 — done

Both closed out: `TestScene::OnSceneBoot` now adds a real `DirectionalLight`, the Tab
handler's dead `"Changing"`-tag lookup is gone (calls `SetColor` on a real material
directly), `Texture::Unbind(slot)` takes and activates a unit, `Material` has a second
texture slot for the specular map, `fragment.glsl` samples it in all three light
functions, and `models/crate.obj` is wired into `TestLayer` with both textures. Redline's
Phases A and B, in full.

## ✅ Tier 1 — done

The known-bug backlog, `ResourceManager::LoadMaterial`, and the `RenderContext`→
`FrameContext` rename are all closed out — see Known Bugs and Open Architecture below.

## ✅ Tier 3 — mostly done (engine capability, not gated on the TCG — see `SCOPE.md`)

Multi-camera/split-screen and Skybox/Skydome are both done — see Open Architecture below
and the Completed section. Sub-mesh support is the one Tier 3 item not started yet.

## 🔜 Next up — per `SCOPE.md`: Tier 2

**Tier 2 (net-new, required for v1 — the real remaining work):** text/UI rendering, a
clickable-region abstraction, threading `FrameContext` into `Layer::OnEvent`/`OnUpdate`,
the 2D/orthographic + picking primitives ported back from Solitaire, and networking
(client-server, home-server-hosted, ENet, ported over the existing home VPN). All of
this used to live under Redline's "optional, not on the roadmap" Phase F — it no longer
is optional. Full detail and reasoning in `SCOPE.md`.

---

## 🐛 Known bugs — 2026-07-13 audit pass (unfixed, ranked by severity)

A full pass over every `Forge`/`Demo` source file, each finding verified against the
actual code (and in two cases against actual compiled behavior) before being logged here.
Nothing in this section has been fixed yet — this is the starting punch list for whoever
picks it up next.

**High — visibly wrong or crash-risk today**

- [ ] **Ruby cube's "spin" orbits instead of spinning in place** (`models/cube.obj`, `Model.cpp:30-37`, `LightDemoLayer.cpp:104-107`) — `cube.obj`'s vertices span `z ∈ [0,20]`, so its local origin sits on one face, not the centroid. `GetModelMatrix()` rotates before translating, so `SetSpin` sweeps the cube's visual center in a circle instead of rotating it in place. Visible in every demo scene right now.
- [x] **Sprint boost doesn't apply to strafing** (`Camera.cpp:117-125`) — fixed: `MoveLeft`/`MoveRight` now scale by `(speed_ + boost_)` like the other four movement methods.
- [x] **Window resize only reaches the active scene** (`Engine.cpp:219`) — fixed, indirectly: `WindowResize` still only updates the shared `FrameContext`'s window size and forwards to the active scene, but `Scene::Resume()` (see the new Suspend/Resume scene-switch lifecycle) now re-syncs every camera's `Viewport` from that shared `FrameContext` whenever a scene becomes active again, so a scene that missed a resize while inactive catches up the moment it's switched back to.
- [ ] **Texture loader picks the wrong GL format for grayscale images** (`Texture.cpp:30`) — any non-4-channel image is treated as `GL_RGB`; a 1- or 2-channel image (e.g. a grayscale roughness map) was only allocated 1–2 bytes/pixel by `stbi_load`, so `glTexImage2D` reads past the buffer.
- [ ] **`debug_error` is a silent no-op in Release builds** (`Utils.hpp:19-31`) — bundled into the same `#ifndef NDEBUG` block as `debug_info`/`debug_warn`, contradicting the documented "always throws" contract. In Release, a missing shader/texture file silently proceeds with `nrChannels`/`width_`/`height_` uninitialized.
- [x] **No cap on light count vs. the UBO's fixed 32-slot capacity** (`Scene.cpp:144-163`, `FrameContext.hpp:38`) — fixed: `AddLight` now checks `lights_.size()` against `fctx_->MAX_LIGHTS` and drops (with a `debug_warn`) instead of pushing past capacity. Unexercised today (every demo scene uses 5 lights).

**Medium — real, latent, or contract-violating**

- [ ] **Shader compile/link failure leaks GL shader objects** (`Shader.cpp:27-49`) — `glDeleteShader`/`glDeleteProgram` cleanup only runs on the success path.
- [ ] **`Material`/`LoadMaterial` never null-check the injected shader** (`Material.cpp:19-26`, `ResourceManager.cpp:48-57`) — a failed shader load cascades into a null-pointer crash far away, inside `Material::Bind()`.
- [ ] **Double-Tab within one frame collapses to a single scene advance** (`Engine.cpp:164`) — `SetScene((current_scene_+1)%...)` reads the stale `current_scene_`, not the already-pending `next_scene_`.
- [ ] **Tab/backtick aren't actually intercepted before reaching the scene** (`Engine.cpp:163-187, 219`) — contradicts the documented "intercepted before forwarding" contract; both fall through to `scene->OnEvent()` like any other key. Harmless today since nothing downstream binds those keys.
- [ ] **`LoadMaterial`'s cache is keyed only by tag, not `(shader, tag)`** (`ResourceManager.cpp:48-57`) — two different shaders sharing a tag string silently collide.
- [ ] **A tween finish-callback that chains `AddTween` would invalidate `UpdateTweens`'s iterators mid-loop** (`Layer.hpp:39-47`, `Tweens.hpp:53`) — latent; nothing calls `SetFinishFunction` yet.
- [ ] **`Viewport::RecomputeAspectRatio` divides by zero on a 0×0 resize** (`Camera.hpp:35-38`) — GLFW can legitimately deliver this on minimize; produces NaN/Inf straight into the projection matrix.

**Low — real but narrow/cosmetic**

- [ ] **`Mesh::Create(filename)` throws instead of returning `nullptr`** (`Mesh.cpp:31-40`) — contradicts its own doc comment and its two sibling `Create()` overloads. Masked today because its one caller (`ResourceManager::LoadMesh`) happens to catch it.
- [ ] **OBJ parser doesn't support negative/relative face indices** (`Mesh.cpp:127-136`) — silently collapses that vertex to the origin instead of resolving it; a whole-face collapse can further produce a NaN normal.
- [ ] **Skydome shader failing to compile leaks one VAO/frame in Debug builds** (`Scene.cpp:95-99`) — never triggered by shaders currently in the repo.
- [ ] **`Shader`/`Texture` own a raw GL handle but never disable copy** (`Shader.hpp`, `Texture.hpp`) — an accidental copy would double-free. Never triggered today (both only ever used via `shared_ptr`).
- [ ] **`LightDemoLayer`'s Q-key handler indexes `materials_[1]` directly instead of a tag lookup** (`LightDemoLayer.cpp:132`) — works today only by `push_back` order coincidence (already flagged in a comment there).
- [ ] **`LoadShader`'s cache key could theoretically collide** (`ResourceManager.cpp:29`) — `vertPath + "||" + fragPath`; no current shader path contains `"||"`.
- [ ] **Malformed `v`/`vn`/`vt` lines silently default missing components to `0.0`** (`Mesh.cpp:69-84`) — no warning, unlike `f` lines which do get a token-count check.
- [ ] **`SetOrthographic`/`SetBoost` accept invalid values with no validation** (`Camera.cpp`) — zero/negative half-height, negative boost. Not hit by any current call site.

---

## Known bugs / issues found in audit

- [x] **Normal matrix computed on GPU** (`vertex.glsl:18`) — fixed: computed once per draw on the CPU (`glm::transpose(glm::inverse(modelMatrix))`) and uploaded as `uniform mat3 uNormalMatrix`; the vertex shader just does `vNormal = uNormalMatrix * aNormal` now. Originally landed in `TestLayer::OnRender`, later moved into base `Layer::Render()` — see the Completed section.
- [x] **OBJ loader drops non-triangle faces** (`Mesh.cpp:81`) — fixed: faces are parsed into a per-face vertex-index list first, then fan-triangulated (`indices[0]` anchors triangles against every consecutive pair) instead of requiring exactly 3 tokens. Assumes convex, planar faces — true for Blender's default export.
- [x] **`assert` in `UpdateRenderContext`** (`Scene.cpp`) — fixed: replaced with `if (cameras_.empty()) { debug_error(...); return; }`, so the check survives in release builds instead of compiling out.
- **`ResourceManager::LoadShader` cache ignores tag** (`ResourceManager.cpp:28`) — not a bug: tags aren't used to differentiate cache entries yet, this is intentional future-proofing until something actually needs per-tag shader variants.
- [x] **`getModelMatrix()` camelCase** (`Model.cpp:30`) — fixed: renamed to `GetModelMatrix()` everywhere (header, implementation, `TestLayer.cpp` call site).
- [x] **`if (!Init())` in Engine constructor doesn't actually guard construction** (`Engine.cpp:10`) — fixed: `debug_error` only throws when `NDEBUG` is unset, so in a release build it was a no-op and the constructor just `return`ed early, leaving the `Engine` half-constructed. The constructor now always `throw`s a plain `std::runtime_error` on `Init()` failure, independent of `NDEBUG` — `debug_error` still logs (and throws again, harmlessly unreachable) in debug builds, but the guard itself no longer depends on it.
- [x] **`Layer.hpp`'s interface doesn't match `CLAUDE.md`'s description** — resolved by decision, not by matching the doc to a pure-virtual `Destroy()`: none of `Suspend()`/`Transition()`/`Destroy()` need to be mandatory on every `Layer`. Suspend is just switching the active `Scene`/`Layer`, `Destroy` is redundant with a subclass's own destructor running via `shared_ptr`, and `Transition` can be implemented however a given `Layer` needs — none of it belongs on the base interface as a forced override. `Destroy()` is commented out of `Layer.hpp` accordingly. (Note: there is no `CLAUDE.md` in this repo currently, so "matches the doc" doesn't apply either way — this line item is now just stale.)
- [x] **`Texture::Unbind()` unbinds whatever texture unit is currently active, not necessarily unit 0** (`Texture.cpp:59`) — fixed: `Unbind(GLuint slot = 0)` now calls `glActiveTexture(GL_TEXTURE0 + slot)` before unbinding.
- [x] **`LightType::Area` has no matching class** (`Light.hpp`) — resolved: enumerator dropped from `LightType` (never implemented, out of scope per `SCOPE.md`'s "area lights" exclusion).
- [x] **`Texture` never sets `GL_UNPACK_ALIGNMENT`** (`Texture.cpp`) — fixed: `glPixelStorei(GL_UNPACK_ALIGNMENT, 1);` added right before `glTexImage2D`.

Fixed in this audit:
- [x] **Back faces rendered** — `glEnable(GL_CULL_FACE)` + `glCullFace(GL_BACK)` added to `Engine::Init` (`Engine.cpp:58`)
- [x] **Fragment shader: no clamp, full ambient** — added `0.1 *` ambient scale and `clamp(...)` on output (`fragment.glsl`)

---

## Open Architecture

- [x] **Rename `RenderContext`** — done: the struct and file are renamed to `FrameContext` (`include/Forge/FrameContext.hpp`, formerly `RenderContext.hpp`), every `Material::Bind`/`Layer::OnRender` call site takes `shared_ptr<FrameContext>`, and `Renderer::GetFrameContext()`/`Scene::UpdateFrameContext()` (formerly `GetRenderContext`/`UpdateRenderContext`) are renamed to match too.

- [x] **`ResourceManager::LoadMaterial`** — done: takes `(shared_ptr<Shader> shader, const string& tag, glm::vec3 ambient = ..., glm::vec3 diffuse = ..., glm::vec3 specular = ..., float shininess = ...)` with averaged defaults across the existing Blinn-Phong presets, caches by tag via the same weak_ptr pattern as `LoadMesh`/`LoadTexture`, and delegates to `Material::Create(shader, tag, ambient, diffuse, specular, shininess)`.

- [x] **Multiple cameras / split-screen** — done (Tier 3, see `SCOPE.md`): `Camera` owns a `Viewport` (normalized `[0,1]` rect + the window's current pixel size, recomputed on resize via `Camera::UpdateViewportSize`/`Viewport::SetWindowSize`, never a stale absolute-pixel rect). `Scene::Render()` loops over every camera in `cameras_` each frame — `ApplyViewport(i)` (`Viewport::Apply()`: `glViewport`+`glScissor`), `UpdateFrameContext(i)`, a scissored depth clear, `DrawBackground()`, then every layer's `Render()` — wrapped in `glEnable`/`glDisable(GL_SCISSOR_TEST)` so one camera's clear can't bleed into another's viewport. `active_camera_` stays purely an input-routing index — every camera in `cameras_` renders every frame regardless of which one is active. `MultiCameraDemoScene` exercises this: a free-fly main camera plus a fixed picture-in-picture overview (`Camera::SetPosition`/`SetYawPitch`, added alongside this).

---

## Next: Shaders + Lighting

To be done in order — vertex shader first (independent), then light struct design, then fragment shader and C++ lights together.

---

### Step 1 — Vertex shader (`shaders/vertex.glsl`) ✅

- [x] Add `layout(location = 1) in vec3 aNormal`
- [x] Add `layout(location = 2) in vec2 aTexCoords`
- [x] Add `out vec3 vNormal`, `out vec3 vFragPos`, `out vec2 vTexCoords`
- [x] Transform normal to world space: `vNormal = mat3(transpose(inverse(uModel))) * aNormal`
- [x] Pass through: `vTexCoords = aTexCoords`, `vFragPos = vec3(uModel * vec4(aPos, 1.0))`

---

### Step 2 — Design the light struct (no code, paper/whiteboard) ✅

- [x] Decide on a max light count — went with 32 (`fragment.glsl`'s `MAX_LIGHTS`), not the suggested 16
- [x] Decide how to encode light type (int: 0=directional, 1=point, 2=spot — matches `LightType` enum) — packed into `GPULight::position.w`
- [x] Account for std140 padding — every `vec3` occupies 16 bytes, not 12; every `GPULight` field is a `vec4` (`position`, `direction`, `color`, `params`, `params2`), no mixed scalar/vec3 arrays
- [x] Decide: pack `uLightCount` into the UBO itself instead of a separate plain `uniform int` — done as `uLightCountHeader` (a `vec4`, count in `.x`), read directly by the fragment shader's loop bound

---

### Step 3 — Fragment uber-shader (`shaders/fragment.glsl`) ✅

- [x] Declare inputs: `vNormal`, `vFragPos`, `vTexCoords`
- [x] Declare `uniform bool uHasTexture`, `uniform sampler2D uTexture`, `uniform vec4 uBaseColor`
- [x] Declare `uniform struct { vec3 ambient; vec3 diffuse; vec3 specular; float shininess; } uMaterial`
- [x] Declare `uniform vec3 uCameraPos`, `uniform float uTime`
- [x] Unlit/texture branch: `baseColor = uHasTexture ? texture(...) : uBaseColor`
- [x] Blinn-Phong calculation — no longer a hardcoded placeholder light, see below
- [x] Replaced the hardcoded light with `layout(std140, binding = 0) uniform LightsBlock` (a `Light lights[MAX_LIGHTS]` array) + a loop over it in `main()`
- [x] Reads the light count from the UBO itself (`uLightCountHeader.x`, no separate `uLightCount` uniform)
- [x] Light color factored into ambient/diffuse/specular in all three of `DirectionLightValue`/`PointLightValue`/`SpotLightValue`
- [x] Branches on light type (directional/point/spot) inside the loop via `switch (int(lights[i].position.w))`
- [x] Attenuation for point and spot lights (`1.0 / (constant + linear*d + quadratic*d*d)`, from `GPULight::params`)
- [x] Cone angle test for spot lights (`coneFactor` in `SpotLightValue`, smoothed between inner/outer cutoff rather than a hard edge)

---

### Step 4 — C++ Light classes ✅

- [x] Abstract base `Light` (`include/Forge/Light.hpp`): `color_` (vec3), `intensity_` (float), `LIGHT_CLASS_TYPE` macro, `LightType` enum (Directional, Point, Spot)
- [x] `DirectionalLight` (`include/Forge/Lights.hpp`): adds `direction_` (vec3), normalised on set
- [x] `PointLight`: adds `position_` (vec3), attenuation (`constant_`, `linear_`, `quadratic_`)
- [x] `SpotLight`: adds `position_`, `direction_`, `innerCutoff_`, `outerCutoff_` (stored as cosines)
- [x] `GPULight` packed struct — ended up in `include/Forge/Light.hpp` itself rather than a separate `GPULight.hpp`, but otherwise as planned: all-`vec4` fields, matches the GLSL `Light` struct in `fragment.glsl` field-for-field
- [x] `virtual GPULight ToGPULight() const = 0;` on `Light`, implemented per subclass in `src/Forge/Lights.cpp` (packing logic lives next to the data it packs, as intended)

---

### Step 5 — UBO wiring in `Scene` ✅

Two distinct kinds of work here — don't conflate them, they run at different frequencies:
- **One-time setup** (e.g. in `OnLoad`/`OnSceneBoot`, once ever): allocate the buffer and bind it to its binding point. After this, *any* shader declaring the matching `layout(std140, binding=0)` block reads it automatically — no per-material or per-layer rebinding, ever.
- **Per-frame refresh** (once per frame, not per layer/material): update the buffer's contents, since light positions/count can change frame to frame.

- [x] `vector<shared_ptr<Light>> lights_` and `AddLight()` on `Scene`
- [x] One-time setup ended up on `RenderContext` instead of `Scene` directly — `RenderContext::CreateLightBuffer()` (`glGenBuffers`/`glBufferData` sized for `MAX_LIGHTS`, `glBindBufferBase(GL_UNIFORM_BUFFER, 0, ...)`), called once from `Engine::Init` right after GLAD loads
- [x] Per-frame refresh: `Scene::LoadLights()` packs `lights_` into a `GPULight` array via `ToGPULight()` and uploads with `glBufferSubData`, called from `Render()` alongside `UpdateRenderContext()`

---

### Step 6 — Integration — done

- [x] `TestLayer` uses `fragment.glsl` and `Material` presets (Gold etc.)
- [x] `TestScene::OnSceneBoot` — adds three `PointLight`s via `AddLight()` (a warm key light + two colored rim lights)
- [x] `TestScene::OnSceneBoot` — a real `DirectionalLight` added; `DirectionLightValue`'s shader path now actually runs against real scene data
- [x] `TestLayer` Tab handler — dead `"Changing"`-tag lookup replaced with a direct `material->SetColor(Forge::Color_A1::RandomColor())` call
- [x] `Light.cpp` *was* needed after all — `ToGPULight()`'s implementations live in `src/Forge/Lights.cpp` (light classes are no longer purely header-only)

---

### Step 7 — Lighting maps (textures) — done

Independent of Steps 2–6 (textures and lighting don't depend on each other), but shares the same "mostly shader work" shape. `Material` already proves this pattern once for the diffuse/base-color slot — this is that same pattern, doubled, for a specular map.

- [x] Procedural crate texture generator (`scripts/generate_crate_textures.py`) + generated `textures/crate_diffuse.png`/`crate_specular.png` (512×512, no external/network assets)
- [x] `models/crate.obj` — UV-unwrapped cube (24 unshared vertices, `vt`/`vn` per face) to pair with the generated textures
- [x] Fixed `Texture::Unbind()`'s active-unit assumption (see Known Bugs) before two texture units are ever bound at once
- [x] `Material`: second texture slot — `specularTexture_` member, `SetSpecularTexture()`, `HasSpecularMap()` (mirrors the existing `texture_`/`SetTexture()`/`HasTexture()`)
- [x] `Material::Bind()`: binds the specular texture to texture unit 1, sets `uHasSpecularMap`/`uSpecularMap`
- [x] `fragment.glsl`: samples `uSpecularMap` in place of `uMaterial.specular` when `uHasSpecularMap` is true, threaded through all three light functions
- [x] Crate model + both textures wired into `TestLayer`, rendered alongside the existing demo materials

---

## Backlog — Tier 3 (see `SCOPE.md`), not gated on the TCG

- **Sub-mesh support** — one OBJ file produces N `(Mesh, Material)` pairs via `usemtl` groups. Requires Material system (done) + OBJ parser update + Scene/Layer wiring. The one Tier 3 item not started yet.

Multi-camera/split-screen and Skybox/Skydome, formerly listed here, are both done — see Open Architecture and Completed above/below.

---

## Lessons from building Solitaire on a copy of this engine

**Update:** this is no longer just a heads-up for "whenever this engine grows toward 2D/UI
content" — per `SCOPE.md`, it's growing that way on purpose, now. Every item below that
isn't already fixed is Tier 2 in `SCOPE.md`'s definition of done, not a someday-maybe.

None of this is implemented here yet. Originally recorded as a heads-up for whenever
this engine grows toward 2D/UI content or gets reused for something beyond the current
3D demo scene. A full Klondike Solitaire (click-and-drag cards, top-down camera) was
built on an AI-finished copy of this exact engine snapshot; these are the gaps/traps
that copy needed patched or worked around that weren't visible from a 3D-only demo
scene.

- **No 2D/orthographic support at all** — `Camera` only ever did perspective. A flat/UI-style scene needs `SetOrthographic(halfHeight)`, and the default `lookAt` world-up `(0,1,0)` degenerates for a straight-down camera (front vector nearly parallel to up, cross product ~0) — needed a `SetUp(vec3)` override to supply a horizontal reference instead.
- **No picking primitives** — click-and-drag needs a screen-to-world ray (`Camera::ScreenPointToRay`, via `inverse(projection * view)`) and ray/AABB + ray/plane intersection tests. None of that existed; `Model` also needed a world-space bounding box (`GetWorldBounds()`) to intersect against.
- **`Mesh` had no way to address a sub-rect of a texture** — needed a `CreateQuad(tag, uvMin, uvMax)` overload for texture-atlas sprites (52 playing cards sharing one atlas image, one quad per card pointing at its own cell).
- **`GL_BLEND` is never enabled anywhere in `Engine::Init`** — real alpha transparency (e.g. a translucent UI overlay/outline) silently does nothing until it's turned on. Costs nothing for existing fully-opaque textures, so there's no real downside to just always enabling it.
- **No animation/tweening utility** — ended up writing a small `Vec3Tween`/`EaseOutQuad` helper (lerp position A→B over a duration, with easing) from scratch for a card-slide animation. Small, generic, reusable — worth having in `Forge` rather than re-deriving per-project.
- **`Layer::OnEvent`/`OnUpdate` aren't given the camera/frame data (`FrameContext`)** — only `OnRender` gets it. Anything that needs view/projection for picking, or delta-time for animation, outside of `OnRender` has to cache whatever `OnRender` last saw. Works fine for a static camera; would break the moment the camera moves and an event fires between frames.
- **No text/UI rendering system** — this is why the current engine has no on-screen score/HUD/prompt of any kind, and Solitaire had to cut scoring/timer features and use a card "bounce" animation instead of a win banner. Worth keeping in mind as a real, currently-missing capability, not just a Solitaire-specific gap.
- **Orthographic cameras have a genuine invisible axis** — animating an object purely along the camera's own view direction is a real position change with *zero* visible pixels under orthographic projection (no perspective falloff to reveal it). Bit a "victory bounce" animation that moved along the wrong axis; only caught by actually looking at a rendered frame, not by checking the position math. Worth remembering for *any* animation added to an orthographic scene later.
- **Coincident flat quads z-fight** — two quads at the exact same depth (e.g. an animated sprite passing through a static marker at the same position) flicker unpredictably. Needed a small manual depth offset to fix; a proper convention (consistent tiny Z-bias for "decorative overlay" quads, or explicit draw-order control) would avoid needing to hand-fix this per case.
- **No way to screenshot/verify a render from outside the process** — every external screenshot tool failed in that sandbox. `glReadPixels` called directly from inside the render loop worked, but had to be re-added as throwaway temporary code every time a visual change needed checking, then removed again before committing. A small supported "step N frames headlessly and dump a framebuffer capture" hook on `Engine` would make this repeatable instead of hand-rolled each time — probably worth adding once this engine's demo scenes get complex enough that "did it compile" stops being enough to trust a change.

**Update — more lessons from a later Solitaire session**, adding undo/redo, a solver-backed "always deal a winnable game" guarantee, on-table icon buttons, and a couple of art/rendering fixes. These weren't visible in the first pass because that pass never resized the window, never added a second interactive UI element, and never needed real alpha edges on non-rectangular art:

- **Window resize is a silent trap the first pass never hit.** `Scene::ResizeCameras()` already exists (built for split-screen — recomputes every camera's aspect ratio from its viewport rect and the window's current size) but nothing calls it automatically; a `Scene` subclass has to remember to call it itself on `WindowResize`. Solitaire's scene never did. It compiled fine, ran fine, looked correct at whatever size it happened to boot at — and only visibly stretched every card the moment a user actually dragged a window corner. Two legitimate fixes exist depending on what a scene wants: auto-reflow (call `ResizeCameras()`) for something split-screen-like, or lock the window itself to a fixed ratio for a fixed-layout 2D scene (added `WindowSpecification::lockAspectRatio`, wired via `glfwSetWindowAspectRatio` in `Engine::Init`, opt-in and off by default). Worth considering whether `Scene`'s own resize handling should default to calling `ResizeCameras()` unless a subclass opts out, rather than requiring every subclass to remember to opt in — doing neither is the current silent default.
- **Still no reusable "clickable region" concept, even after building actual UI buttons.** Every hit-test in Solitaire — the stock pile, then an Undo button, then a Reset button — is the same hand-written "is this world point within half-width/half-height of this center" check, copy-pasted with new numbers each time. Sharpens the earlier "no text/UI system" lesson: the gap isn't just text rendering, it's that there's no lightweight "clickable sprite" or hit-region abstraction at all, so every new interactive element means re-deriving the same fixed-rect logic by hand.
- **`GL_BLEND` being on doesn't mean transparency is fully safe — depth writes are a separate, still-open trap.** A blended fragment still writes its own depth to the depth buffer by default, alpha or no alpha, unless a draw explicitly disables depth writes for that pass. This didn't visibly break anything in Solitaire only by luck of geometry (new transparent card corners are small and rarely overlap another sprite that needed to show through at that exact pixel) — a scene with denser overlapping transparency (particles, layered UI) would hit real occlusion bugs from this. Distinct from the z-fight lesson above (that's two coincident depths flickering; this is occlusion ordering from a depth *write* that alpha doesn't suppress).
- **(Process, not code) Same-hue backgrounds can hide a broken alpha mask.** Solitaire's card atlas had solid green squares baked into its rounded corners, left over from the source art's own canvas color — unnoticed for a while because the table felt is *also* green, so a broken opaque corner and a correctly transparent one looked nearly identical in ad hoc testing. Worth a habit for any future art-import pipeline: preview a new alpha texture against a deliberately mismatched color before trusting it, not against the color it'll actually ship over.

---

## Completed

- **Skybox/Skydome implemented** — `Scene::DrawSkyboxBackground()`/`DrawSkydomeBackground()` (previously `// TODO` stubs) are real now, both procedural (no textures/cubemaps). Skybox: a hardcoded 36-vertex unit cube (`Mesh::Create` from raw floats, no OBJ), `shaders/skybox_vertex.glsl`/`skybox_fragment.glsl` — vertex shader strips the view matrix's translation (`mat3(view)`) so the cube always surrounds the camera, and pins depth to the far plane (`gl_Position = vec4(pos.xy, pos.w, pos.w)`); fragment shader is a zenith/horizon/ground gradient plus a sun disc aimed opposite the scene's directional light. Skydome: no mesh at all — `shaders/skydome_vertex.glsl` builds a full-screen triangle purely from `gl_VertexID`, `skydome_fragment.glsl` reconstructs the view ray per-pixel via `inverse(projection*view)`, independently-tuned gradient+horizon haze. Both wrapped in `glDepthFunc(GL_LEQUAL)`/`glDepthMask(GL_FALSE)`; skybox additionally disables face culling for its one draw (camera is inside the cube). `Scene` gained a real destructor to release the skydome's raw VAO. New public `Scene::SetSkyboxBackground()`/`SetSkydomeBackground()` setters alongside the existing `SetBackgroundColor()`.
- **`Camera::SetPosition`/`SetYawPitch` added** — direct setters bypassing `CameraMove`/`Update`'s edge-triggered movement and `Rotate()`'s mouse-look path, for a fixed/scripted camera (e.g. a picture-in-picture overview) that never receives input.
- **`LightDemoScene`/`MultiCameraDemoScene` content fully redesigned** — `LightDemoLayer` now builds a floor (hardcoded quad `Vertex` data, no OBJ, same "raw vertex array" pattern as the skybox cube) plus five "stations" laid out along it: one cube per Blinn-Phong preset (Gold/Silver/Ruby/Emerald), each paired with a dedicated light in `OnSceneBoot` positioned/aimed directly at it so that light's effect dominates its own station instead of five lights blending uniformly over a scattered pile, plus a fifth station for the textured crate. `LightDemoScene` uses the new Skybox; `MultiCameraDemoScene` (same `LightDemoLayer` content, two cameras) uses the new Skydome, and its picture-in-picture camera is now a genuinely different elevated overview (`SetPosition`/`SetYawPitch`) instead of sharing the main camera's default pose.
- **`Engine::SetScene(GLint index)` + deferred scene switching** — bounds-checked, switches which registered scene is active; applied at the top of `Engine::Run()`'s loop via a `next_scene_` member (not immediately), so `current_scene_` can never change mid-frame between `Update()` and `RenderScene()`. `Tab` cycles scenes by default (`Engine::RaiseEvent`: `SetScene((current_scene_ + 1) % scenes_.size())`) — this is why `LightDemoLayer`'s own randomize-color handler moved off `Tab` onto `Q`.
- **`Scene::ResetMouse()` added, fixes a real camera jerk on scene switch** — scenes aren't recreated on switch (each is constructed once by `AddScene` and lives forever in `scenes_`), so a scene's camera keeps whatever `last_x_`/`last_y_` mouse-tracking state it had the last time it was active — stale the moment a different scene has been active in the meantime. `ResetMouse()` (`final`, non-virtual on the base `Scene`, since the reset itself isn't scene-specific) resets the active camera's tracking; `Engine::Run()` calls it on the newly-active scene exactly when the deferred switch (above) is applied.
- **`App` renamed to `Demo`, `Test` namespace renamed to `Demo`** — `include/App`→`include/Demo`, `src/App`→`src/Demo`, every `namespace Test`/`Test::`/`"App/...` reference. `TestScene`/`TestLayer` renamed to `LightDemoScene`/`LightDemoLayer` in the process (see the content-redesign entry above for what they do now); `MultiCameraDemoScene` added alongside them.
- **`BUILD_DEMO` CMake option + `make demo`/`make lib`** — `option(BUILD_DEMO ... ON)` wraps the `OpenGL_App` target (default on, matching prior always-built behavior); `-DBUILD_DEMO=OFF` builds `libEngineCore.a` alone with zero demo-layer code compiled — the actual "just the library" path the `make lib` idea from earlier in this project's history had been parked on. Sticky like `SHOW_FPS`/`LOG_EVENTS` until toggled or `make clean`. `make demo` configures `BUILD_DEMO=ON`, builds, and runs `OpenGL_App`; `make lib` configures `BUILD_DEMO=OFF` and builds only `EngineCore`.
- **Input events decoupled from GLFW** — `include/Forge/Keys.hpp` adds `Forge::Key`/`Forge::MouseButton`, replacing the raw GLFW key/button codes `KeyEvent`/`MouseButtonEvent` used to carry. Their values are deliberately wired to match GLFW's own `GLFW_KEY_*`/`GLFW_MOUSE_BUTTON_*` numbering (not derived from `<GLFW/glfw3.h>` — `Keys.hpp` itself has zero GLFW dependency), so `EventHandler` translates with a plain `static_cast` instead of a lookup table; `EventHandler` remains the one place in Forge allowed to know GLFW's numbering. `test/test_keys.cpp` guards every value against GLFW's real constants — caught a real bug immediately (`Tab`/`Enter`/`Escape`/`Backspace` had been set to their ASCII control-code values instead of GLFW's actual codes). Closes the last of the two closeable gaps found auditing `~/Solitaire` for raw OpenGL/GLFW leaking into app-facing code (the other, hand-authored `.glsl` shaders, is an accepted structural exception, not a gap).
- **`Layer::Render()`/`OnRender()` split** — `Layer::Render()` (new, non-virtual, engine-owned) now does the material-bind + per-model `uModel`/`uNormalMatrix` upload + `Draw()` loop that used to live in `TestLayer::OnRender`; `OnRender()` is now just a per-layer hook for anything layer-specific, empty in `TestLayer`. Moves raw shader-uniform-name knowledge out of app code — the other closeable gap from the Solitaire audit.
- **`debug_error` no longer discards the exception message** — `include/Utils.hpp`: builds the message into a local `std::ostringstream` and reuses it for both the console print and `std::runtime_error`'s constructor, instead of the message only ever reaching `std::cout` and the exception always carrying `""`. Fixes a real regression affecting every `catch (const std::exception&) { ...e.what()... }` call site (`Shader`/`Texture`/`Mesh`/`ResourceManager`/`Lights`/`Material`).
- **`Shader.cpp` compile bug fixed** — `debug_error("Shader compile error (" << type << "): " << infoLog)` didn't compile (a `<<`-chain can't be passed directly as `std::runtime_error`'s constructor argument); fixed as part of the `debug_error` fix above.
- **Namespace/class rename** — `Core` namespace renamed to `Forge` (directories `include/Core`→`include/Forge`, `src/Core`→`src/Forge`, every `Core::` qualifier and `#include "Core/...` path); the `Application` class renamed to `Engine` (`Application.hpp`/`.cpp` → `Engine.hpp`/`.cpp`, every method/doc-comment reference). `ApplicationSpecification`/`appName` intentionally left as-is (not part of the ask). `EngineCore`/`EngineCore_tests` CMake target names also intentionally left as-is — a separate decision from the namespace/class rename.
- Documentation prep pass over `Forge` — Doxygen + Doxygen Awesome CSS set up (`docs/Doxyfile`, theme vendored at `docs/doxygen-awesome-css/` pinned to `v2.4.2`, run via `doxygen docs/Doxyfile`); every public class/member in `include/Forge/*.hpp` (19 files) got `@brief`/`@param`/`@return`/`@warning` comments; `Scene.hpp` and `Lights.hpp` split into declaration-only headers + new `Scene.cpp`/`Lights.cpp` (no behavior change, verified by a clean rebuild).
- Follow-up Doxygen pass — closed the remaining gaps the first pass missed (`Material`'s 9 Blinn-Phong preset factories, `Shader`'s 9 uniform setters, `RenderContext::CreateLightBuffer`/`BindLightBuffer` and several of its members, `Camera::GetFOV`/`SetBoost`), and documented `App/` (`TestLayer`, `TestScene`, `InputConfig`'s `key_map`), which the first pass had explicitly left for later.
- **Test suite added** — `EngineCore_tests` (opt-in via `-DBUILD_TESTS=ON`, `make test` builds and runs it in one step) now has 42 tests across 5 files: `test_camera.cpp` (FOV clamping/sequencing, WASD movement composition, view/projection matrix sanity, mouse-look's first-call-primes-only behavior), `test_mesh.cpp` (all four OBJ face-index forms, vertex dedup, n-gon fan-triangulation, degenerate/unknown-line handling, the flat-normal-generation branch), `test_lights.cpp` (per-subclass `LIGHT_CLASS_TYPE` identity, direction normalization, `SpotLight`'s degree→cosine cutoff conversion, `ToGPULight()`'s exact slot layout per light type), `test_model.cpp` (`GetModelMatrix()`'s translate/rotate/scale composition, including that scale must apply before rotation), and `test_colors.cpp` (`RandomColor()`'s bounds/variety invariants). `Mesh::Create(filename)`'s OBJ-parsing/flat-normal-generation logic was extracted into a new `Mesh::ParseObjFile(std::istream&, sourceName)` specifically to make it unit-testable without a GL context — everything GPU-related (`Mesh`'s GPU upload, `Texture`, `Shader`, `ResourceManager::Load*`, `Engine`/`Scene`/`Renderer`) still needs a live context and has no coverage yet.
- `Camera::SetBoost(GLfloat)` — adds a speed bonus on top of base movement speed for every direction (e.g. a sprint key); wired to Left Ctrl in `TestScene::OnEvent`. Not yet covered by `test_camera.cpp`.
- Procedurally-generated textured crate asset prepared for the upcoming texturing/lighting-maps work — `textures/crate_diffuse.png` + `textures/crate_specular.png` (512x512, wood-with-metal-braces look, generated by `scripts/generate_crate_textures.py`, no external/network assets). Not yet wired into `Material`/`ResourceManager`/`TestLayer` — `models/crate.obj` (a UV-unwrapped cube to pair with these textures) is still to be written by hand.
- Texture class (`include/Forge/Texture.hpp`, `src/Forge/Texture.cpp`) — private constructor, `Create(path)` factory, `Bind(slot)`, `Unbind()`, RAII destructor, stb_image load with vertical flip, GL_RGB/GL_RGBA format detection
- Material class (`include/Forge/Material.hpp`, `src/Forge/Material.cpp`) — private constructor, `Create(shader, tag)` factory, `Bind(ctx)` sets all shader uniforms + binds texture, Blinn-Phong coefficients (ambient/diffuse/specular/shininess), base color, `GetShader()`, setters
- Layer refactor — `shaderModels_`/`shaders_` replaced by `materialModels_`/`materials_`; `OnRender` now calls `material->Bind(ctx)` once per bucket
- `TestLayer` updated — uses Material, tag-based lookup via `GetTag()`, Tab handler uses `GetShader()` as temporary workaround until `lit.frag` exists
- `ResourceManager` updated — `LoadTexture` returns `shared_ptr<Texture>` with weak_ptr cache; `LoadMaterial` signature added (not yet implemented)
- `Colors.hpp` moved from `include/App/` to `include/Forge/`, namespace changed from `Test::Color_A1` to `Forge::Color_A1`; all call sites updated
- FPS counter changed to 1-second rolling average (was instantaneous `1/delta_time`)
- stb_image added at `include/stb_image/stb_image.h`
- Vertex attributes correctly wired in `Mesh::setup()` at locations 0 (pos), 1 (normal), 2 (texCoords)
- `Forge::Scene` fully abstract — `OnEvent`, `OnUpdate`, `OnMouseCapture`, `OnSceneBoot` pure virtual
- Forge/App separation complete — engine has zero compile-time dependency on app-layer code
- Scene construction moved to `main.cpp`; `Engine` no longer includes app-layer headers
- `Scene::OnLoad(rmanager, rctx)` stores context then calls `OnSceneBoot()` — scene owns its init sequence
- `Solitare` namespace renamed to `Test`
- `key_map` moved to `include/Forge/Camera.hpp` as `inline const`
- Renderer calls `scene->Render()` instead of iterating layers directly
- Shader uniform locations cached on first use
- `DEBUG` macros gated on `NDEBUG`; event logging gated on `LOG_EVENTS`
- Double color-buffer clear per frame fixed
- Camera jump on cursor recapture fixed
- `WindowResizeEvent` firing and aspect ratio update fixed
- `fctx_->aspect_ratio_` member ordering fix (was garbage on startup)
- `VSync` flag correctly applied after `glfwMakeContextCurrent`
