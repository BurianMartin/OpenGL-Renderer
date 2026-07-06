# Renderer Roadmap

Tracks open bugs, design decisions, and implementation plans.

---

## 🔜 Next up — two Step 6 loose ends, then lighting maps (textures)

Two related efforts, both the user's own implementation (Claude guides/reviews, doesn't write this code — see CLAUDE.md). Detailed step-by-step lists live under "Next: Shaders + Lighting" below (Step 6 for the lighting loose ends, Step 7 for textures); this is the at-a-glance status.

**Lighting UBO — done:** the whole pipeline actually works now (commit "Wire the multi-light UBO into the renderer") — vertex shader normals/UVs (Step 1), the light struct design (Step 2), `GPULight` + per-subclass `ToGPULight()` (Step 4), `Scene`'s one-time UBO alloc + per-frame refresh (Step 5), and the fragment shader's real multi-light loop with attenuation + cone falloff (Step 3). The checkboxes under Steps 2/3/4/5 below were still unticked after that commit landed — ticked now to match reality.
**Lighting UBO — left (Step 6):** `TestScene::OnSceneBoot` only adds `PointLight`s, no `DirectionalLight` yet; `TestLayer`'s Tab handler is still dead code looking for a `"Changing"`-tagged material that no longer exists (should call `material->SetColor(...)` on a real material instead).

**Lighting maps (textures) — done:** procedural crate texture generator + generated `textures/crate_diffuse.png`/`crate_specular.png` (not yet wired anywhere); `Texture`/`ResourceManager::LoadTexture` and `Material`'s existing one-texture-slot pattern are already proven via the diffuse/base-color map.
**Lighting maps (textures) — left:** `models/crate.obj` (UV-unwrapped cube), a second `Material` texture slot for the specular map (same pattern as the existing diffuse slot), shader sampling, and fixing `Texture::Unbind()`'s active-unit assumption before juggling two texture units at once.

---

## Known bugs / issues found in audit

- [ ] **Normal matrix computed on GPU** (`vertex.glsl:18`) — `mat3(transpose(inverse(uModel)))` runs per-vertex. Should be computed on CPU and passed as `uniform mat3 uNormalMatrix`. Low priority now, but will hurt when many models are on screen.
- [ ] **OBJ loader drops non-triangle faces** (`Mesh.cpp:81`) — quads and n-gons silently skipped. Blender exports quads by default. Always export with "Triangulate Faces", or add a fan-triangulation step to the loader.
- [ ] **`assert` in `UpdateRenderContext`** (now `Scene.cpp` — moved out of the header in the doc-prep pass) — compiles out in release, turning a missing-camera error into UB. Replace with `debug_error`.
- [ ] **`ResourceManager::LoadShader` cache ignores tag** (`ResourceManager.cpp:28`) — same vert+frag paths with different tags return the first cached shader with the wrong tag.
- [ ] **`getModelMatrix()` camelCase** (`Model.cpp:30`) — inconsistent with all other PascalCase getters. Rename to `GetModelMatrix()`.
- [ ] **`if (!Init())` in Application constructor doesn't actually guard construction** (`Application.cpp:10`) — corrected from an earlier audit pass: `Init()` *can* return `false` (GLFW init failure, window creation failure, GLAD load failure all return `false` before throwing via `debug_error`), so the check isn't dead code. But in a release build `debug_error` is a no-op and the constructor just `return`s early — leaving the `Application` half-constructed rather than preventing its use. Needs the constructor to actually stop construction (e.g. throw unconditionally) on `Init()` failure.
- [ ] **`Layer.hpp`'s interface doesn't match `CLAUDE.md`'s description** — `CLAUDE.md` says `Transition()`/`Suspend()`/`Destroy()` "have default empty implementations; only override if needed," but in the actual header `Transition()`/`Suspend()` are commented out entirely (not part of the interface at all) and `Destroy()` is pure virtual (`= 0`), forcing every Layer subclass to implement it even if it does nothing. Either update `CLAUDE.md` to match reality, or give `Destroy()` a default empty body and uncomment `Transition()`/`Suspend()` with defaults, whichever was actually intended.
- [ ] **`Texture::Unbind()` unbinds whatever texture unit is currently active, not necessarily unit 0** (`Texture.cpp:59`) — it calls `glBindTexture(GL_TEXTURE_2D, 0)` without an `glActiveTexture` call first, so it acts on whatever unit was last made active (typically whatever slot the most recent `Bind()` used). Fine today since only one texture slot is ever bound, but worth fixing before adding a second (specular map) slot so `Unbind()` can't accidentally target the wrong unit.
- [ ] **`LightType::Area` has no matching class** (`Light.hpp`) — the enum reserves a `Area` value but there is no `AreaLight` subclass, no `LIGHT_CLASS_TYPE(Area)` usage anywhere. Either implement it or drop the enumerator until it's needed.
- [ ] **`Texture` never sets `GL_UNPACK_ALIGNMENT`** (`Texture.cpp`, just before the `glTexImage2D` call) — GL defaults to an alignment of 4, meaning it assumes each row of pixel data is padded to a multiple of 4 bytes. stb_image's buffer is tightly packed with no such padding, so any image whose `width * channels` isn't a multiple of 4 gets loaded with GL silently reading past the real data (undefined behavior — in practice, a segfault or garbled texture). Confirmed as a real, currently-live bug: it's what caused a hard crash loading a differently-sized texture atlas in a Solitaire game built on a copy of this engine — no image tried against `Texture` so far in *this* repo happens to have hit the alignment, but it's latent, not fixed. One-line fix: `glPixelStorei(GL_UNPACK_ALIGNMENT, 1);` right before `glTexImage2D`.

Fixed in this audit:
- [x] **Back faces rendered** — `glEnable(GL_CULL_FACE)` + `glCullFace(GL_BACK)` added to `Application::Init` (`Application.cpp:58`)
- [x] **Fragment shader: no clamp, full ambient** — added `0.1 *` ambient scale and `clamp(...)` on output (`fragment.glsl`)

---

## Open Architecture

- [ ] **Rename `RenderContext`** — currently holds camera matrices, time, delta_time, aspect_ratio. The name implies render target only. Rename to something like `FrameContext` or `SceneContext` everywhere.

- [ ] **`ResourceManager::LoadMaterial`** — signature exists but has no implementation. Needs a decision on parameters (material isn't a file — needs shader + tag at minimum). Implement when the Layer integration is fully settled.

- [ ] **Multiple cameras / split-screen** — `Scene::cameras_` and `active_camera_` already support multiple cameras; the render side does not. Needs a viewport rect per camera, `glViewport` calls, and `Renderer::RenderScene` running once per active camera.

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

### Step 6 — Integration — partially done, two items left

- [x] `TestLayer` uses `fragment.glsl` and `Material` presets (Gold etc.)
- [x] `TestScene::OnSceneBoot` — adds three `PointLight`s via `AddLight()` (a warm key light + two colored rim lights)
- [ ] `TestScene::OnSceneBoot` — still no `DirectionalLight` added; only `PointLight`s exist in the scene right now, so `DirectionLightValue`'s code path in the shader has never actually been exercised against real scene data
- [ ] `TestLayer` Tab handler — still dead code looking for a `"Changing"`-tagged material that no longer exists (`std::ranges::find_if` fails, logs `"Failed to find Changing material"`, does nothing). Update to `material->SetColor(Core::Color_A1::RandomColor())` on one of the real materials instead.
- [x] `Light.cpp` *was* needed after all — `ToGPULight()`'s implementations live in `src/Core/Lights.cpp` (light classes are no longer purely header-only)

---

### Step 7 — Lighting maps (textures)

Independent of Steps 2–6 (textures and lighting don't depend on each other), but shares the same "mostly shader work" shape. `Material` already proves this pattern once for the diffuse/base-color slot — this is that same pattern, doubled, for a specular map.

- [x] Procedural crate texture generator (`scripts/generate_crate_textures.py`) + generated `textures/crate_diffuse.png`/`crate_specular.png` (512×512, no external/network assets)
- [ ] `models/crate.obj` — UV-unwrapped cube (24 unshared vertices, `vt`/`vn` per face) to pair with the generated textures
- [ ] Fix `Texture::Unbind()`'s active-unit assumption (see Known Bugs) before two texture units are ever bound at once
- [ ] `Material`: add a second texture slot — `specularTexture_` member, `SetSpecularTexture()`, `HasSpecularMap()` (mirrors the existing `texture_`/`SetTexture()`/`HasTexture()`)
- [ ] `Material::Bind()`: bind the specular texture to a second texture unit (slot 1), set `uHasSpecularMap`/`uSpecularMap`
- [ ] `fragment.glsl`: sample `uSpecularMap` in place of `uMaterial.specular` when `uHasSpecularMap` is true
- [ ] Wire the crate model + both textures into `TestLayer` (or a new layer) to actually see it rendered

---

## Backlog

- **Sub-mesh support** — one OBJ file produces N `(Mesh, Material)` pairs via `usemtl` groups. Requires Material system (done) + OBJ parser update + Scene/Layer wiring. Natural next feature after lighting.
- **Skybox / Skydome** — `DrawSkyboxBackground()` and `DrawSkydomeBackground()` are stubbed in `Scene`; finish after textures are working.

---

## Lessons from building Solitaire on a copy of this engine

None of this is implemented here — this repo is still mid-lighting/textures.
Recorded now, while it's fresh, as a heads-up for whenever this engine grows
toward 2D/UI content or gets reused for something beyond the current 3D
demo scene. A full Klondike Solitaire (click-and-drag cards, top-down
camera) was built on an AI-finished copy of this exact engine snapshot;
these are the gaps/traps that copy needed patched or worked around that
weren't visible from a 3D-only demo scene.

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
