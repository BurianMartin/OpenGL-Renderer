# Renderer Roadmap

Tracks open bugs, design decisions, and implementation plans.

---

## Open Bugs

- [ ] **Vertex normals and texCoords uploaded but unused** — `Mesh::setup()` wires attributes at locations 1 (normal) and 2 (texCoords) but `vertex.glsl` only declares `aPos`. Normals and UV data are silently discarded by the driver every frame. Prerequisite for all lighting and texture work.

---

## Open Architecture

- [ ] **Multiple cameras / split-screen** — `Scene::cameras_` and `active_camera_` already support multiple cameras; the render side does not. Needs a viewport rect per camera, `glViewport` calls to partition the window, and `Renderer::RenderScene` running once per active camera with its own view/projection. `RenderContext` updated per camera per pass.

- [ ] **Rename `RenderContext`** — currently holds camera matrices, time, delta_time, aspect_ratio. The name implies render target only. Rename to something like `FrameContext` or `SceneContext` everywhere.

---

## Implementation Plan: Textures + Lighting + Materials

To be done in one pass — all three touch the same files. Doing them separately means writing the vertex shader, Material class, and fragment shaders twice.

Estimated time: 14–24 hours.

---

### Phase 1 — Vertex shader groundwork

1. Update `shaders/vertex.glsl`:
   - Add `layout(location = 1) in vec3 aNormal`
   - Add `layout(location = 2) in vec2 aTexCoords`
   - Pass both to the fragment stage as `out vec3 vNormal` and `out vec2 vTexCoords`
   - Transform normal to world space: `vNormal = mat3(transpose(inverse(uModel))) * aNormal`

---

### Phase 2 — Texture system

2. Add `stb_image.h` to the project (single-header library, drop into `include/vendor/`). Define `STB_IMAGE_IMPLEMENTATION` in exactly one `.cpp` file.

3. Create `include/Core/Texture.hpp` + `src/Core/Texture.cpp`:
   - Private constructor, `Texture::Create(path)` factory — consistent with `Mesh` and `Shader`
   - Load image with stb_image, upload via `glTexImage2D`, generate mipmaps
   - `Bind(GLuint slot)` — calls `glActiveTexture(GL_TEXTURE0 + slot)` then `glBindTexture`
   - `Unbind()`
   - Stores `ID_`, `width_`, `height_`

4. Add `ResourceManager::LoadTexture(path)` — same `weak_ptr` cache pattern as `LoadMesh` and `LoadShader`.

---

### Phase 3 — Light class hierarchy

5. Create `include/Core/Light.hpp`:
   - Abstract base `Light`: `color_` (vec3), `intensity_` (float), pure virtual `GetType()`
   - `PointLight`: adds `position_` (vec3), attenuation constants (`constant_`, `linear_`, `quadratic_`)
   - `DirectionalLight`: adds `direction_` (vec3), no position
   - `SpotLight`: adds `position_` (vec3), `direction_` (vec3), inner and outer cone angles

6. Define the **std140-aligned GPU struct** — must match the GLSL UBO block exactly:
   - `vec3` pads to `vec4` under std140 — every vec3 field needs a trailing float padding
   - Include a `type` int field so the shader can branch per light
   - Define a max light count constant (e.g. 32) used in both C++ and GLSL

---

### Phase 4 — UBO setup

7. Add `vector<shared_ptr<Light>> lights_` and `AddLight(shared_ptr<Light>)` to `Core::Scene`.

8. Create the UBO in `Scene::OnSceneBoot`:
   - `glGenBuffers`, `glBindBuffer(GL_UNIFORM_BUFFER)`
   - `glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_)` — binding point 0 is the lights block
   - Allocate buffer storage for max light count upfront

9. In `Scene::UpdateRenderContext()`, pack current lights into the std140 struct and upload via `glBufferSubData`. Also write the active light count into the buffer.

---

### Phase 5 — Material class

10. Create `include/Core/Material.hpp` + `src/Core/Material.cpp`:
    - Owns `shared_ptr<Shader>`
    - Owns `vector<pair<shared_ptr<Texture>, GLuint>>` (texture + slot index)
    - Owns per-material uniform values (base color `vec4`, etc.)
    - `Material::Create(shader)` factory
    - `AddTexture(texture, slot)`
    - `Bind(shared_ptr<RenderContext>)` — calls `shader->Use()`, binds textures, sets camera uniforms from rctx (view, projection, camera_position, time)

11. Update `Model`:
    - Add `shared_ptr<Material> material_`
    - Add `SetMaterial(shared_ptr<Material>)`

12. Update `Layer`:
    - Replace `shaderModels_` (`map<shared_ptr<Shader>, vector<shared_ptr<Model>>>`) with `materialModels_` (`map<shared_ptr<Material>, vector<shared_ptr<Model>>>`)
    - Replace `shaders_` vector with `materials_` vector
    - Update `OnRender`: iterate `materialModels_`, call `material->Bind(rctx)`, set `uModel` per model, draw

---

### Phase 6 — Fragment shaders

13. Create `shaders/lit_color.frag` — Blinn-Phong, solid base color, no texture:
    - Declare the lights UBO block (`layout(std140, binding = 0)`) matching the C++ struct exactly
    - Loop over active lights, branch on type (point / directional / spot)
    - Output: ambient + diffuse + specular

14. Create `shaders/lit_textured.frag` — same as above but samples `uniform sampler2D uTexture` for the base color.

15. Keep `shaders/solid_color.frag` and `shaders/time_color.glsl` as-is for unlit use.

---

### Phase 7 — Integration and test

16. Update `TestScene::OnSceneBoot`: add at least one `PointLight` and one `DirectionalLight` via `AddLight()`.

17. Update `TestLayer`: replace the raw shader+model map with a `Material`, assign a texture if one is available, use `lit_color.frag` or `lit_textured.frag`.

18. Add stb_image and all new source files to `CMakeLists.txt`.

19. Build, run, verify lighting and textures are correct.

---

## Completed

Kept for reference.

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
