# Renderer Roadmap

Tracks open bugs, design decisions, and implementation plans.

---

## 🔜 Next session — Finish lighting

The lighting pipeline is partially done. The fragment shader has working Blinn-Phong with a **hardcoded placeholder light** (`normalize(2, 3, 1.5)`). The C++ `Light`, `PointLight`, `DirectionalLight`, `SpotLight` classes exist but are not yet connected to the renderer.

**Remaining steps in order:**
1. Design `GPULight` packed struct with std140 padding (Step 2)
2. Replace hardcoded light in `fragment.glsl` with UBO loop (Step 3)
3. Wire the UBO into `Scene` — `AddLight()`, `glGenBuffers`, upload per frame (Step 5)
4. Add real lights in `TestScene::OnSceneBoot()` (Step 6)

---

## Known bugs / issues found in audit

- [ ] **Normal matrix computed on GPU** (`vertex.glsl:18`) — `mat3(transpose(inverse(uModel)))` runs per-vertex. Should be computed on CPU and passed as `uniform mat3 uNormalMatrix`. Low priority now, but will hurt when many models are on screen.
- [ ] **OBJ loader drops non-triangle faces** (`Mesh.cpp:81`) — quads and n-gons silently skipped. Blender exports quads by default. Always export with "Triangulate Faces", or add a fan-triangulation step to the loader.
- [ ] **`assert` in `UpdateRenderContext`** (`Scene.hpp:43`) — compiles out in release, turning a missing-camera error into UB. Replace with `debug_error`.
- [ ] **`ResourceManager::LoadShader` cache ignores tag** (`ResourceManager.cpp:28`) — same vert+frag paths with different tags return the first cached shader with the wrong tag.
- [ ] **`getModelMatrix()` camelCase** (`Model.cpp:30`) — inconsistent with all other PascalCase getters. Rename to `GetModelMatrix()`.
- [ ] **Dead `if (!Init())` in Application constructor** (`Application.cpp:10`) — `Init()` never returns false, it always throws. Check is dead code.

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

### Step 2 — Design the light struct (no code, paper/whiteboard)

- [ ] Decide on a max light count (e.g. 32)
- [ ] Decide how to encode light type (int: 0=directional, 1=point, 2=spot — matches `LightType` enum)
- [ ] Account for std140 padding — every `vec3` occupies 16 bytes, not 12; use `vec4` in the GPU struct

---

### Step 3 — Fragment uber-shader (`shaders/fragment.glsl`) — partially done

- [x] Declare inputs: `vNormal`, `vFragPos`, `vTexCoords`
- [x] Declare `uniform bool uHasTexture`, `uniform sampler2D uTexture`, `uniform vec4 uBaseColor`
- [x] Declare `uniform struct { vec3 ambient; vec3 diffuse; vec3 specular; float shininess; } uMaterial`
- [x] Declare `uniform vec3 uCameraPos`, `uniform float uTime`
- [x] Unlit/texture branch: `baseColor = uHasTexture ? texture(...) : uBaseColor`
- [x] Blinn-Phong calculation with hardcoded directional light at `normalize(1, 2, 0.5)` as placeholder
- [ ] Replace hardcoded light with `layout(std140, binding = 0) uniform LightsBlock` + loop over lights
- [ ] Add `uniform int uLightCount`
- [ ] Add light color to ambient/diffuse/specular calculations
- [ ] Branch on light type (directional vs point vs spot) inside the loop
- [ ] Add attenuation for point and spot lights
- [ ] Add cone angle test for spot lights

---

### Step 4 — C++ Light classes ✅

- [x] Abstract base `Light` (`include/Core/Light.hpp`): `color_` (vec3), `intensity_` (float), `LIGHT_CLASS_TYPE` macro, `LightType` enum (Directional, Point, Spot)
- [x] `DirectionalLight` (`include/Core/Lights.hpp`): adds `direction_` (vec3), normalised on set
- [x] `PointLight`: adds `position_` (vec3), attenuation (`constant_`, `linear_`, `quadratic_`)
- [x] `SpotLight`: adds `position_`, `direction_`, `innerCutoff_`, `outerCutoff_` (stored as cosines)
- [ ] Define `GPULight` packed struct with std140 padding (vec3 → vec4, trailing int for type) — must match the GLSL struct from Step 3 exactly

---

### Step 5 — UBO wiring in `Scene`

- [ ] Add `vector<shared_ptr<Light>> lights_` and `AddLight()` to `Scene`
- [ ] Add `GLuint lightUBO_` member
- [ ] Create and allocate the UBO in `OnSceneBoot`: `glGenBuffers`, `glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightUBO_)`
- [ ] In `UpdateRenderContext()`: pack lights into `GPULight` array, upload via `glBufferSubData`, upload `uLightCount`

---

### Step 6 — Integration

- [x] `TestLayer` uses `fragment.glsl` and `Material` presets (Gold etc.)
- [ ] `TestScene::OnSceneBoot` — add a `PointLight` and a `DirectionalLight` via `AddLight()`
- [ ] `TestLayer` Tab handler — update to `material->SetColor(...)` (currently dead code looking for "Changing" tag)
- [ ] Confirm `Light.cpp` not needed (all light classes are header-only)

---

## Backlog

- **Sub-mesh support** — one OBJ file produces N `(Mesh, Material)` pairs via `usemtl` groups. Requires Material system (done) + OBJ parser update + Scene/Layer wiring. Natural next feature after lighting.
- **Skybox / Skydome** — `DrawSkyboxBackground()` and `DrawSkydomeBackground()` are stubbed in `Scene`; finish after textures are working.

---

## Completed

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
