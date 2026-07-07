# Renderer Roadmap

Tracks open bugs, design decisions, and implementation plans. **See `SCOPE.md` first**
for what this engine is actually for and how much of this file's backlog is in scope —
several items below (multi-camera/split-screen, the whole Backlog section) are
deliberately deferred, not gating v1, and Redline's phase order now follows `SCOPE.md`'s
priority rather than the order it was originally written in.

---

## ✅ Step 6 & Step 7 — done

Both closed out: `TestScene::OnSceneBoot` now adds a real `DirectionalLight`, the Tab
handler's dead `"Changing"`-tag lookup is gone (calls `SetColor` on a real material
directly), `Texture::Unbind(slot)` takes and activates a unit, `Material` has a second
texture slot for the specular map, `fragment.glsl` samples it in all three light
functions, and `models/crate.obj` is wired into `TestLayer` with both textures. Redline's
Phases A and B, in full.

## 🔜 Next up — per `SCOPE.md`: finish Tier 1, then Tier 2

**Tier 1 (close the existing gap — small, mechanical, ~3-4 hrs total):** the known-bug
backlog below, `ResourceManager::LoadMaterial`, the `RenderContext`→`FrameContext`
rename. Multi-camera/split-screen is **deferred** — see `SCOPE.md`, not needed for the
TCG.

**Tier 2 (net-new, required for v1 — the real remaining work):** text/UI rendering, a
clickable-region abstraction, threading `FrameContext` into `Layer::OnEvent`/`OnUpdate`,
the 2D/orthographic + picking primitives ported back from Solitaire, and networking
(client-server, home-server-hosted, ENet, ported over the existing home VPN). All of
this used to live under Redline's "optional, not on the roadmap" Phase F — it no longer
is optional. Full detail and reasoning in `SCOPE.md`.

---

## Known bugs / issues found in audit

- [x] **Normal matrix computed on GPU** (`vertex.glsl:18`) — fixed: computed once per draw on the CPU in `TestLayer::OnRender` (`glm::transpose(glm::inverse(modelMatrix))`) and uploaded as `uniform mat3 uNormalMatrix`; the vertex shader just does `vNormal = uNormalMatrix * aNormal` now.
- [x] **OBJ loader drops non-triangle faces** (`Mesh.cpp:81`) — fixed: faces are parsed into a per-face vertex-index list first, then fan-triangulated (`indices[0]` anchors triangles against every consecutive pair) instead of requiring exactly 3 tokens. Assumes convex, planar faces — true for Blender's default export.
- [x] **`assert` in `UpdateRenderContext`** (`Scene.cpp`) — fixed: replaced with `if (cameras_.empty()) { debug_error(...); return; }`, so the check survives in release builds instead of compiling out.
- **`ResourceManager::LoadShader` cache ignores tag** (`ResourceManager.cpp:28`) — not a bug: tags aren't used to differentiate cache entries yet, this is intentional future-proofing until something actually needs per-tag shader variants.
- [x] **`getModelMatrix()` camelCase** (`Model.cpp:30`) — fixed: renamed to `GetModelMatrix()` everywhere (header, implementation, `TestLayer.cpp` call site).
- [ ] **`if (!Init())` in Application constructor doesn't actually guard construction** (`Application.cpp:10`) — corrected from an earlier audit pass: `Init()` *can* return `false` (GLFW init failure, window creation failure, GLAD load failure all return `false` before throwing via `debug_error`), so the check isn't dead code. But in a release build `debug_error` is a no-op and the constructor just `return`s early — leaving the `Application` half-constructed rather than preventing its use. Needs the constructor to actually stop construction (e.g. throw unconditionally) on `Init()` failure.
- [x] **`Layer.hpp`'s interface doesn't match `CLAUDE.md`'s description** — resolved by decision, not by matching the doc to a pure-virtual `Destroy()`: none of `Suspend()`/`Transition()`/`Destroy()` need to be mandatory on every `Layer`. Suspend is just switching the active `Scene`/`Layer`, `Destroy` is redundant with a subclass's own destructor running via `shared_ptr`, and `Transition` can be implemented however a given `Layer` needs — none of it belongs on the base interface as a forced override. `Destroy()` is commented out of `Layer.hpp` accordingly. (Note: there is no `CLAUDE.md` in this repo currently, so "matches the doc" doesn't apply either way — this line item is now just stale.)
- [x] **`Texture::Unbind()` unbinds whatever texture unit is currently active, not necessarily unit 0** (`Texture.cpp:59`) — fixed: `Unbind(GLuint slot = 0)` now calls `glActiveTexture(GL_TEXTURE0 + slot)` before unbinding.
- [x] **`LightType::Area` has no matching class** (`Light.hpp`) — resolved: enumerator dropped from `LightType` (never implemented, out of scope per `SCOPE.md`'s "area lights" exclusion).
- [x] **`Texture` never sets `GL_UNPACK_ALIGNMENT`** (`Texture.cpp`) — fixed: `glPixelStorei(GL_UNPACK_ALIGNMENT, 1);` added right before `glTexImage2D`.

Fixed in this audit:
- [x] **Back faces rendered** — `glEnable(GL_CULL_FACE)` + `glCullFace(GL_BACK)` added to `Application::Init` (`Application.cpp:58`)
- [x] **Fragment shader: no clamp, full ambient** — added `0.1 *` ambient scale and `clamp(...)` on output (`fragment.glsl`)

---

## Open Architecture

- [ ] **Rename `RenderContext`** — currently holds camera matrices, time, delta_time, aspect_ratio. The name implies render target only. Rename to something like `FrameContext` or `SceneContext` everywhere.

- [ ] **`ResourceManager::LoadMaterial`** — signature exists but has no implementation. Needs a decision on parameters (material isn't a file — needs shader + tag at minimum). Implement when the Layer integration is fully settled.

- [ ] **Multiple cameras / split-screen** — *deferred, see `SCOPE.md`: not needed for the TCG, not gating v1.* `Scene::cameras_` and `active_camera_` already support multiple cameras; the render side does not. Needs a viewport rect per camera, `glViewport` calls, and `Renderer::RenderScene` running once per active camera.

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

- [x] Abstract base `Light` (`include/Core/Light.hpp`): `color_` (vec3), `intensity_` (float), `LIGHT_CLASS_TYPE` macro, `LightType` enum (Directional, Point, Spot)
- [x] `DirectionalLight` (`include/Core/Lights.hpp`): adds `direction_` (vec3), normalised on set
- [x] `PointLight`: adds `position_` (vec3), attenuation (`constant_`, `linear_`, `quadratic_`)
- [x] `SpotLight`: adds `position_`, `direction_`, `innerCutoff_`, `outerCutoff_` (stored as cosines)
- [x] `GPULight` packed struct — ended up in `include/Core/Light.hpp` itself rather than a separate `GPULight.hpp`, but otherwise as planned: all-`vec4` fields, matches the GLSL `Light` struct in `fragment.glsl` field-for-field
- [x] `virtual GPULight ToGPULight() const = 0;` on `Light`, implemented per subclass in `src/Core/Lights.cpp` (packing logic lives next to the data it packs, as intended)

---

### Step 5 — UBO wiring in `Scene` ✅

Two distinct kinds of work here — don't conflate them, they run at different frequencies:
- **One-time setup** (e.g. in `OnLoad`/`OnSceneBoot`, once ever): allocate the buffer and bind it to its binding point. After this, *any* shader declaring the matching `layout(std140, binding=0)` block reads it automatically — no per-material or per-layer rebinding, ever.
- **Per-frame refresh** (once per frame, not per layer/material): update the buffer's contents, since light positions/count can change frame to frame.

- [x] `vector<shared_ptr<Light>> lights_` and `AddLight()` on `Scene`
- [x] One-time setup ended up on `RenderContext` instead of `Scene` directly — `RenderContext::CreateLightBuffer()` (`glGenBuffers`/`glBufferData` sized for `MAX_LIGHTS`, `glBindBufferBase(GL_UNIFORM_BUFFER, 0, ...)`), called once from `Application::Init` right after GLAD loads
- [x] Per-frame refresh: `Scene::LoadLights()` packs `lights_` into a `GPULight` array via `ToGPULight()` and uploads with `glBufferSubData`, called from `Render()` alongside `UpdateRenderContext()`

---

### Step 6 — Integration — done

- [x] `TestLayer` uses `fragment.glsl` and `Material` presets (Gold etc.)
- [x] `TestScene::OnSceneBoot` — adds three `PointLight`s via `AddLight()` (a warm key light + two colored rim lights)
- [x] `TestScene::OnSceneBoot` — a real `DirectionalLight` added; `DirectionLightValue`'s shader path now actually runs against real scene data
- [x] `TestLayer` Tab handler — dead `"Changing"`-tag lookup replaced with a direct `material->SetColor(Core::Color_A1::RandomColor())` call
- [x] `Light.cpp` *was* needed after all — `ToGPULight()`'s implementations live in `src/Core/Lights.cpp` (light classes are no longer purely header-only)

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

## Backlog — deferred, see `SCOPE.md`

Neither of these is needed for the TCG; not gating v1. Left here rather than deleted in case a future 3D project revives them.

- **Sub-mesh support** — one OBJ file produces N `(Mesh, Material)` pairs via `usemtl` groups. Requires Material system (done) + OBJ parser update + Scene/Layer wiring. Natural next feature after lighting, if this were still a 3D-demo-first project.
- **Skybox / Skydome** — `DrawSkyboxBackground()` and `DrawSkydomeBackground()` are stubbed in `Scene`; finish after textures are working, if ever needed.

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
- **`GL_BLEND` is never enabled anywhere in `Application::Init`** — real alpha transparency (e.g. a translucent UI overlay/outline) silently does nothing until it's turned on. Costs nothing for existing fully-opaque textures, so there's no real downside to just always enabling it.
- **No animation/tweening utility** — ended up writing a small `Vec3Tween`/`EaseOutQuad` helper (lerp position A→B over a duration, with easing) from scratch for a card-slide animation. Small, generic, reusable — worth having in `Core` rather than re-deriving per-project.
- **`Layer::OnEvent`/`OnUpdate` aren't given the camera/frame data (`FrameContext`)** — only `OnRender` gets it. Anything that needs view/projection for picking, or delta-time for animation, outside of `OnRender` has to cache whatever `OnRender` last saw. Works fine for a static camera; would break the moment the camera moves and an event fires between frames.
- **No text/UI rendering system** — this is why the current engine has no on-screen score/HUD/prompt of any kind, and Solitaire had to cut scoring/timer features and use a card "bounce" animation instead of a win banner. Worth keeping in mind as a real, currently-missing capability, not just a Solitaire-specific gap.
- **Orthographic cameras have a genuine invisible axis** — animating an object purely along the camera's own view direction is a real position change with *zero* visible pixels under orthographic projection (no perspective falloff to reveal it). Bit a "victory bounce" animation that moved along the wrong axis; only caught by actually looking at a rendered frame, not by checking the position math. Worth remembering for *any* animation added to an orthographic scene later.
- **Coincident flat quads z-fight** — two quads at the exact same depth (e.g. an animated sprite passing through a static marker at the same position) flicker unpredictably. Needed a small manual depth offset to fix; a proper convention (consistent tiny Z-bias for "decorative overlay" quads, or explicit draw-order control) would avoid needing to hand-fix this per case.
- **No way to screenshot/verify a render from outside the process** — every external screenshot tool failed in that sandbox. `glReadPixels` called directly from inside the render loop worked, but had to be re-added as throwaway temporary code every time a visual change needed checking, then removed again before committing. A small supported "step N frames headlessly and dump a framebuffer capture" hook on `Application` would make this repeatable instead of hand-rolled each time — probably worth adding once this engine's demo scenes get complex enough that "did it compile" stops being enough to trust a change.

**Update — more lessons from a later Solitaire session**, adding undo/redo, a solver-backed "always deal a winnable game" guarantee, on-table icon buttons, and a couple of art/rendering fixes. These weren't visible in the first pass because that pass never resized the window, never added a second interactive UI element, and never needed real alpha edges on non-rectangular art:

- **Window resize is a silent trap the first pass never hit.** `Scene::ResizeCameras()` already exists (built for split-screen — recomputes every camera's aspect ratio from its viewport rect and the window's current size) but nothing calls it automatically; a `Scene` subclass has to remember to call it itself on `WindowResize`. Solitaire's scene never did. It compiled fine, ran fine, looked correct at whatever size it happened to boot at — and only visibly stretched every card the moment a user actually dragged a window corner. Two legitimate fixes exist depending on what a scene wants: auto-reflow (call `ResizeCameras()`) for something split-screen-like, or lock the window itself to a fixed ratio for a fixed-layout 2D scene (added `WindowSpecification::lockAspectRatio`, wired via `glfwSetWindowAspectRatio` in `Application::Init`, opt-in and off by default). Worth considering whether `Scene`'s own resize handling should default to calling `ResizeCameras()` unless a subclass opts out, rather than requiring every subclass to remember to opt in — doing neither is the current silent default.
- **Still no reusable "clickable region" concept, even after building actual UI buttons.** Every hit-test in Solitaire — the stock pile, then an Undo button, then a Reset button — is the same hand-written "is this world point within half-width/half-height of this center" check, copy-pasted with new numbers each time. Sharpens the earlier "no text/UI system" lesson: the gap isn't just text rendering, it's that there's no lightweight "clickable sprite" or hit-region abstraction at all, so every new interactive element means re-deriving the same fixed-rect logic by hand.
- **`GL_BLEND` being on doesn't mean transparency is fully safe — depth writes are a separate, still-open trap.** A blended fragment still writes its own depth to the depth buffer by default, alpha or no alpha, unless a draw explicitly disables depth writes for that pass. This didn't visibly break anything in Solitaire only by luck of geometry (new transparent card corners are small and rarely overlap another sprite that needed to show through at that exact pixel) — a scene with denser overlapping transparency (particles, layered UI) would hit real occlusion bugs from this. Distinct from the z-fight lesson above (that's two coincident depths flickering; this is occlusion ordering from a depth *write* that alpha doesn't suppress).
- **(Process, not code) Same-hue backgrounds can hide a broken alpha mask.** Solitaire's card atlas had solid green squares baked into its rounded corners, left over from the source art's own canvas color — unnoticed for a while because the table felt is *also* green, so a broken opaque corner and a correctly transparent one looked nearly identical in ad hoc testing. Worth a habit for any future art-import pipeline: preview a new alpha texture against a deliberately mismatched color before trusting it, not against the color it'll actually ship over.

---

## Completed

- Documentation prep pass over `Core` — Doxygen + Doxygen Awesome CSS set up (`docs/Doxyfile`, theme vendored at `docs/doxygen-awesome-css/` pinned to `v2.4.2`, run via `doxygen docs/Doxyfile`); every public class/member in `include/Core/*.hpp` (19 files) got `@brief`/`@param`/`@return`/`@warning` comments; `Scene.hpp` and `Lights.hpp` split into declaration-only headers + new `Scene.cpp`/`Lights.cpp` (no behavior change, verified by a clean rebuild). `App/` is not documented yet — see `docs/README.md` for coverage notes.
- Procedurally-generated textured crate asset prepared for the upcoming texturing/lighting-maps work — `textures/crate_diffuse.png` + `textures/crate_specular.png` (512x512, wood-with-metal-braces look, generated by `scripts/generate_crate_textures.py`, no external/network assets). Not yet wired into `Material`/`ResourceManager`/`TestLayer` — `models/crate.obj` (a UV-unwrapped cube to pair with these textures) is still to be written by hand.
- Texture class (`include/Core/Texture.hpp`, `src/Core/Texture.cpp`) — private constructor, `Create(path)` factory, `Bind(slot)`, `Unbind()`, RAII destructor, stb_image load with vertical flip, GL_RGB/GL_RGBA format detection
- Material class (`include/Core/Material.hpp`, `src/Core/Material.cpp`) — private constructor, `Create(shader, tag)` factory, `Bind(ctx)` sets all shader uniforms + binds texture, Blinn-Phong coefficients (ambient/diffuse/specular/shininess), base color, `GetShader()`, setters
- Layer refactor — `shaderModels_`/`shaders_` replaced by `materialModels_`/`materials_`; `OnRender` now calls `material->Bind(ctx)` once per bucket
- `TestLayer` updated — uses Material, tag-based lookup via `GetTag()`, Tab handler uses `GetShader()` as temporary workaround until `lit.frag` exists
- `ResourceManager` updated — `LoadTexture` returns `shared_ptr<Texture>` with weak_ptr cache; `LoadMaterial` signature added (not yet implemented)
- `Colors.hpp` moved from `include/App/` to `include/Core/`, namespace changed from `Test::Color_A1` to `Core::Color_A1`; all call sites updated
- FPS counter changed to 1-second rolling average (was instantaneous `1/delta_time`)
- stb_image added at `include/stb_image/stb_image.h`
- Vertex attributes correctly wired in `Mesh::setup()` at locations 0 (pos), 1 (normal), 2 (texCoords)
- `Core::Scene` fully abstract — `OnEvent`, `OnUpdate`, `OnMouseCapture`, `OnSceneBoot` pure virtual
- Core/App separation complete — engine has zero compile-time dependency on app-layer code
- Scene construction moved to `main.cpp`; `Application` no longer includes app-layer headers
- `Scene::OnLoad(rmanager, rctx)` stores context then calls `OnSceneBoot()` — scene owns its init sequence
- `Solitare` namespace renamed to `Test`
- `key_map` moved to `include/Core/Camera.hpp` as `inline const`
- Renderer calls `scene->Render()` instead of iterating layers directly
- Shader uniform locations cached on first use
- `DEBUG` macros gated on `NDEBUG`; event logging gated on `LOG_EVENTS`
- Double color-buffer clear per frame fixed
- Camera jump on cursor recapture fixed
- `WindowResizeEvent` firing and aspect ratio update fixed
- `rctx_->aspect_ratio_` member ordering fix (was garbage on startup)
- `VSync` flag correctly applied after `glfwMakeContextCurrent`
