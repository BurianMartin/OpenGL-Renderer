# Renderer Roadmap

Tracks open bugs, design decisions, and implementation plans.

---

## Open Architecture

- [ ] **Rename `RenderContext`** — currently holds camera matrices, time, delta_time, aspect_ratio. The name implies render target only. Rename to something like `FrameContext` or `SceneContext` everywhere.

- [ ] **`ResourceManager::LoadMaterial`** — signature exists but has no implementation. Needs a decision on parameters (material isn't a file — needs shader + tag at minimum). Implement when the Layer integration is fully settled.

- [ ] **Multiple cameras / split-screen** — `Scene::cameras_` and `active_camera_` already support multiple cameras; the render side does not. Needs a viewport rect per camera, `glViewport` calls, and `Renderer::RenderScene` running once per active camera.

---

## Next: Shaders + Lighting

To be done in order — vertex shader first (independent), then light struct design, then fragment shader and C++ lights together.

---

### Step 1 — Vertex shader (`shaders/vertex.glsl`)

- [ ] Add `layout(location = 1) in vec3 aNormal`
- [ ] Add `layout(location = 2) in vec2 aTexCoords`
- [ ] Add `out vec3 vNormal` and `out vec2 vTexCoords`
- [ ] Transform normal to world space: `vNormal = mat3(transpose(inverse(uModel))) * aNormal`
- [ ] Pass through: `vTexCoords = aTexCoords`

---

### Step 2 — Design the light struct (no code, paper/whiteboard)

- [ ] Decide what fields each light type needs (position, direction, color, intensity, attenuation, cone angles)
- [ ] Decide on a max light count (e.g. 32)
- [ ] Decide how to encode light type (int: 0=point, 1=directional, 2=spot)
- [ ] Account for std140 padding — every `vec3` occupies 16 bytes, not 12

---

### Step 3 — Fragment uber-shader (`shaders/lit.frag`)

- [ ] Declare `uniform bool uLit` and `uniform bool uHasTexture`
- [ ] Declare `uniform sampler2D uTexture`
- [ ] Declare `uniform vec4 uBaseColor`
- [ ] Declare `uniform struct { vec3 ambient; vec3 diffuse; vec3 specular; float shininess; } uMaterial` — matches what `Material::Bind` already sets
- [ ] Declare the lights UBO block: `layout(std140, binding = 0) uniform LightsBlock` — struct layout must match Step 2 exactly
- [ ] Declare `uniform int uLightCount`
- [ ] Write unlit branch: output `uBaseColor` (or texture sample if `uHasTexture`)
- [ ] Write Blinn-Phong lit branch: loop over lights, branch on type, sum ambient + diffuse + specular

---

### Step 4 — C++ Light classes (`include/Core/Light.hpp`)

- [ ] Abstract base `Light`: `color_` (vec3), `intensity_` (float), pure virtual `GetType()`
- [ ] `PointLight`: adds `position_` (vec3), attenuation (`constant_`, `linear_`, `quadratic_`)
- [ ] `DirectionalLight`: adds `direction_` (vec3)
- [ ] `SpotLight`: adds `position_`, `direction_`, `innerCutoff_`, `outerCutoff_`
- [ ] Define `GPULight` packed struct with std140 padding (vec3 → vec4, trailing int for type) — must match the GLSL struct from Step 3 exactly

---

### Step 5 — UBO wiring in `Scene`

- [ ] Add `vector<shared_ptr<Light>> lights_` and `AddLight()` to `Scene`
- [ ] Add `GLuint lightUBO_` member
- [ ] Create and allocate the UBO in `OnSceneBoot` (after GL context exists): `glGenBuffers`, `glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightUBO_)`
- [ ] In `UpdateRenderContext()`: pack lights into `GPULight` array, upload via `glBufferSubData`, upload `uLightCount`

---

### Step 6 — Integration

- [ ] `TestScene::OnSceneBoot` — load `lit.frag` instead of `solid_color.glsl`, add a `PointLight` and a `DirectionalLight` via `AddLight()`
- [ ] `TestLayer` constructor — create material with the new shader, optionally `SetTexture` if a test image is available
- [ ] `TestLayer` Tab handler — change to `material->SetColor(...)` now that `uBaseColor` is in the shader (currently goes through `GetShader()->SetVec4("triangle_color", ...)` as a workaround)
- [ ] Add `Light.cpp` to `CMakeLists.txt` if needed

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
