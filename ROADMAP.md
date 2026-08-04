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

## ✅ Tier 3 — done (engine capability, not gated on the TCG — see `SCOPE.md`)

Multi-camera/split-screen, Skybox/Skydome, and sub-mesh support are all done — see Open
Architecture below and the Completed section.

## ✅ Text/UI rendering, clickable-region abstraction, `Layer::OnUpdate` FrameContext — done

`Forge::Font`/`Forge::Text` (stb_truetype baked-atlas rendering), `Forge::ClickableRegion`/
`Forge::Button`/`Forge::UILayer` (a real 2D clickable-region/UI-element abstraction), and
`Layer::OnUpdate(shared_ptr<FrameContext>)` (matching `OnEvent`'s existing signature) are
all implemented and exercised end-to-end: `DebugOverlayLayer`'s F3 overlay now shows a live
frame-time/FPS readout alongside its wireframe boxes, and every demo scene has a working
"Randomize Gold" `Forge::Button` wired through a `Forge::UILayer`. See "Next: Text/UI
Rendering" below for the implementation notes (kept for the design rationale) and Known
Bugs for one real gap this surfaced (`Button` click routing under `CursorMode::Captured`).

## 🔜 Next up — per `SCOPE.md`: Tier 2

**Tier 2 (net-new, required for v1 — the one item left):** networking (client-server,
home-server-hosted, ENet, ported over the existing home VPN). Everything else Tier 2 used
to track — text/UI rendering, the clickable-region abstraction, `Layer::OnUpdate`'s
`FrameContext`, and the 2D/orthographic + picking primitives — is done, see above and Open
Architecture below. Full detail and reasoning in `SCOPE.md`.

---

## 🐛 Known bugs — 2026-07-13 + 2026-08-03 audit passes (2026-08-04: closed out — 31/33 fixed, 2 deliberate non-fixes)

A full pass over every `Forge`/`Demo` source file, each finding verified against the
actual code before being logged here, then independently re-verified against current code
in a follow-up pass the same day (2026-08-03: zero false positives). **2026-08-04: commit
`8722888` fixed 31 of these 33 entries in one pass** (verified again, this time against
the fix itself, via four parallel checks) — one of those 31 (`Viewport::RecomputeAspectRatio`'s
divide-by-zero guard) was attempted but didn't actually work on first pass; corrected the
same day, see its entry below. The remaining 2 are deliberate, reasonable non-fixes
(Double-Tab, `Model`'s null `mesh_` check), also called out below. **Every entry in this
section is now either fixed or a deliberate accepted tradeoff — nothing left unaddressed.**

**High — visibly wrong or crash-risk today**

- [x] **Click-to-pick uses the wrong camera's view/projection in any multi-camera scene** — fixed: `Scene::Render()` now re-syncs `fctx_` to `active_camera_` (`UpdateFrameContext(active_camera_)`, `Scene.cpp:266`) right after the per-camera loop, so anything reading `fctx_` afterward (chiefly click-to-pick during the next `glfwPollEvents`) sees the active camera's matrices, not whichever camera rendered last. Landed together with the UILayer/DebugOverlayLayer entry below — same root cause, see that entry for the `Layer::RenderScreenSpace()` mechanism both fixes share.
- [x] **`Forge::Button`/`Forge::UILayer` can't be meaningfully clicked while the cursor is `Captured`** (`UILayer.hpp:OnEvent`) — fixed: confirmed first that the click event itself is never gated on cursor mode (`EventHandler.cpp`'s `glfwSetMouseButtonCallback` fires unconditionally on press/release), so the actual bug was only ever that `ev.GetX()/GetY()` is GLFW's virtual mouse-look accumulator while `Captured`, not a real screen pixel — the same problem `Layer::GetClickedOnObj()` already works around for 3D picking (aims from the window's center pixel instead), except a fixed 2D button has no equivalent "what am I looking at" substitute position, so there's nothing sensible to test against. `UILayer::OnEvent` now checks `ctx->cursor_mode_` (the same field `GetClickedOnObj` already reads) and skips button hit-testing entirely while `Captured`, returning `true` (pass-through, not consumed) so the click event still reaches whatever's behind it (e.g. 3D picking). Deliberately fixed at the consumer (`UILayer`), not by sanitizing coordinates at the event source (`EventHandler`) — keeps `ClickableRegion` a generic, `FrameContext`-independent primitive and avoids a magic out-of-bounds sentinel value. Scene-level "switch to `Normal` cursor mode automatically for UI-heavy scenes" is a separate, still-open design decision left to whoever builds the next UI-heavy scene.
- [x] **Ruby cube's "spin" orbits instead of spinning in place** — fixed: `models/cube.obj`'s Z vertices now run `-10..10` (centered on the origin) instead of `[0,20]`, so the local origin sits at the true centroid; `LightDemoLayer.cpp` compensates with a `+0.25` Z offset on every station position (10 units of recentering × the 0.025 display scale) so nothing visibly moved on-screen.
- [x] **Sprint boost doesn't apply to strafing** (`Camera.cpp:117-125`) — fixed: `MoveLeft`/`MoveRight` now scale by `(speed_ + boost_)` like the other four movement methods.
- [x] **Window resize only reaches the active scene** (`Scene.hpp:123-131`, `Resume()`) — fixed, indirectly: `Scene::Resume()` (the Suspend/Resume scene-switch lifecycle) re-syncs every camera's `Viewport` from the shared `FrameContext` whenever a scene becomes active again, so a scene that missed a resize while inactive catches up the moment it's switched back to. This now genuinely is engine-level behavior end to end — see the next High-tier-turned-fixed entry below (previously listed as a separate Medium item): `AdjustViewport()`'s call moved from `main.cpp` app policy into `Engine::RaiseEvent`'s own fixed bookkeeping, so both halves of window-resize handling now live in the engine, not split between engine and app.
- [x] **Texture loader picks the wrong GL format for grayscale images** — fixed: `Texture.cpp` now switches on `nrChannels` (1→`GL_RED`, 2→`GL_RG`, 4→`GL_RGBA`, else `GL_RGB`) instead of assuming anything non-4-channel is `GL_RGB`.
- [x] **`debug_error` is a silent no-op in Release builds** — fixed: the `#else` (`NDEBUG`) branch in `Utils.hpp` now `throw`s a `std::runtime_error` too, not just logs. Bonus, same commit: all three macros (`debug_info`/`debug_warn`/`debug_error`) are now `do { ... } while (0)`-wrapped in both branches, closing the separate unguarded-macro Low-tier entry below too.
- [x] **No cap on light count vs. the UBO's fixed 32-slot capacity** (`Scene.cpp:159-167`, `FrameContext.hpp:38`) — fixed: `AddLight` now checks `lights_.size()` against `fctx_->MAX_LIGHTS` and drops (with a `debug_warn`) instead of pushing past capacity. Unexercised today (every demo scene uses 5 lights).

**Medium — real, latent, or contract-violating**

- [x] **Shader compile/link failure leaks GL shader objects** — fixed: `Shader.cpp` now calls `glDeleteShader`/`glDeleteProgram` on every failure branch (vertex-fail deletes the vertex shader; fragment-fail deletes both; link-fail deletes both shaders plus the program), not just the success path.
- [x] **`Material`/`LoadMaterial` never null-check the injected shader** — fixed: `ResourceManager::LoadMaterial` now checks `if (!shader) { debug_warn(...); return nullptr; }` before doing anything else; `Material`'s own constructor warns on a null shader too.
- [ ] **Double-Tab within one frame collapses to a single scene advance** (`Engine::NextScene`, `Engine.cpp:243-245`) — **deliberate non-fix, confirmed reasonable 2026-08-04.** `SetScene((current_scene_+1)%...)` still reads the stale `current_scene_`, not the already-pending `next_scene_` — genuinely unchanged. Left as-is on purpose: worst case is a double-tap skips a scene instead of cycling twice, a cosmetic quirk with no crash/correctness impact, not worth the churn to fix.
- [x] **Tab/backtick aren't actually intercepted before reaching the scene** — fixed, as a side effect of the `Engine::HandleEvent`/global-callback rework (see Open Architecture below): `main.cpp`'s registered handler returns `false` (consumed) for Escape/Tab/GraveAccent, so `Engine::RaiseEvent` no longer forwards those key events to the active scene at all.
- [x] **`LoadMaterial`'s cache is keyed only by tag, not `(shader, tag)`** — fixed: the cache key is now `to_string(shader.get()) + ":" + tag` — includes the shader pointer, so two different shaders sharing a tag string can no longer collide.
- [x] **A tween finish-callback that chains `AddTween` would invalidate `UpdateTweens`'s iterators mid-loop** — fixed: `UpdateTweens` (`Layer.hpp:45-55`) now snapshots `tweens_.size()` before the loop and indexes `0..count-1` instead of range-for, so a finish callback calling `AddTween()` mid-loop can't invalidate iterators (a same-frame-added tween is simply deferred to the next frame instead).
- [x] **`Viewport::RecomputeAspectRatio` divides by zero on a 0×0 resize** (`Camera.hpp:38`) — first attempt (2026-08-04) checked the wrong thing (`window_height_ + height_ != 0`, a sum, when the real divisor two lines down is the *product* `window_height_ * height_`) and didn't actually guard the bug's own scenario (`window_height_ == 0`, `height_` defaulting to `1.0f`, sums to `1.0 ≠ 0` and still divides by zero). Corrected same day to `if (window_height_ != 0 && height_ != 0)`, which actually guards both factors of the real divisor.
- [x] **`Forge::UILayer`/`DebugOverlayLayer`'s screen-space text redraws once per camera instead of once per frame in a multi-camera scene** — fixed, same mechanism as the click-to-pick entry above (same root cause: `Scene::Render()`'s per-camera loop treating shared per-frame state as if it were per-camera). `Layer::Render()` no longer calls `OnRender()` at all — a new `Layer::RenderScreenSpace()` (`Layer.hpp:206-210`) does, and `Scene::Render()` invokes it exactly once per frame after the whole per-camera loop finishes (`Scene.cpp:274-276`), with the GL viewport reset to the full window first. Neither `UILayer` nor `DebugOverlayLayer` needed any change themselves — the fix lives entirely in the shared `Layer`/`Scene` base.
- [x] **`DirectionalLight`/`SpotLight` never guard against a zero-length direction vector before `normalize()`** — fixed: all four sites (both constructors, both `SetDirection`s) now guard `if (direction != glm::vec3(0.0f)) { direction_ = glm::normalize(direction); }` — a zero vector is simply ignored instead of producing NaN.
- [x] **`Mesh`'s move constructor/move-assignment silently drop `boundsMin`/`boundsMax`** — fixed: the move constructor's init-list and the move-assignment body both now explicitly copy `boundsMin`/`boundsMax`.
- [ ] **`Model::Draw()`/`Model::GetWorldBounds()` unconditionally dereference `mesh_` with no null check** (`Model.cpp`) — **deliberate non-fix, confirmed reasonable 2026-08-04.** Still unconditional, genuinely unchanged. Left as-is on purpose: a null check on every `Draw()`/`GetWorldBounds()` call — the hot per-frame/per-click-test path — to guard a case the test suite itself only ever exercises via transform-only setters on `Model(nullptr)`, never `Draw()`/`GetWorldBounds()` on one, is a defensible cost/benefit call, not an oversight. `Model`'s constructor still accepts any `shared_ptr<Mesh>` including `nullptr` with no assertion — if this ever bites for real, that's the place to add the guard.
- [x] **`Vec3Tween` never validates `duration_ > 0`** — fixed: the constructor now rejects a non-positive `duration` outright (`if (duration <= 0.0f) { duration_ = 1.0f; debug_warn(...); }`), so `NormalizedT()`'s division never sees a zero denominator in the first place.
- [x] **`WindowResize` handling is app-level policy, not the engine-level bookkeeping `CLAUDE.md` documents it as** — fixed, and cleanly (not duplicated): `Engine::RaiseEvent` now has an explicit `WindowResize` branch calling `AdjustViewport()` directly, positioned as fixed bookkeeping right alongside `WindowLostFocus`. `main.cpp`'s old `case Forge::EventType::WindowResize:` block is gone entirely (falls through to `default: return true;`, which still forwards to the active scene for camera aspect-ratio updates) — no double-call risk.
- [x] **`WindowLostFocusEvent` never reaches the app-settable global handler or the active scene's `OnEvent`** — fixed: the `WindowLostFocus` branch in `Engine::RaiseEvent` no longer `return`s after clearing the held-key table — it now falls through to the same `HandleEvent`/scene-dispatch path every other event type reaches.
- [x] **`Button`/`ClickableRegion` click callbacks are captured via a raw `this` pointer with no lifetime tie to the owning `Layer`** — fixed, and it genuinely closes the risk rather than relocating it: the "Randomize Gold" button's click callback now captures a `std::weak_ptr<Forge::Material>` to the Gold material by value and calls `.lock()` inside the lambda, instead of capturing `this`. This removes the dependency on `LightDemoLayer`'s lifetime entirely — if the `Layer` were destroyed while the `Button`/`UILayer` holding this callback lived on, `.lock()` just returns `nullptr` and the click silently no-ops instead of dereferencing a dangling pointer.

**Low — real but narrow/cosmetic**

- [x] **`Mesh::Create(filename)` throws instead of returning `nullptr`** — fixed: both GPU-uploading factory overloads now wrap construction in `try`/`catch`, `debug_warn` + `return nullptr` on failure, matching the documented contract and the sibling `Create()` overloads.
- [x] **OBJ parser doesn't support negative/relative face indices** — fixed: negative indices are now resolved relative to the current vertex count (`if (vIdx < 0) vIdx = positions.size() + vIdx + 1;`) before the range check, per the OBJ spec, instead of collapsing to the origin. New regression test added: `MeshTest.NegativeFaceIndicesResolveRelativeToCurrentCount` (98 tests total now).
- [x] **Skydome shader failing to compile leaks one VAO/frame in Debug builds** — fixed: a `skydomeInitAttempted_` flag now gates the `glGenVertexArrays` block to run exactly once, even after a failed shader load, instead of re-entering and re-leaking every subsequent frame.
- [x] **`Shader`/`Texture` own a raw GL handle but never disable copy** — fixed: both classes now `= delete` their copy constructor and copy assignment.
- [x] **`SpotLight` never validates `innerCutoff <= outerCutoff`** — fixed: the constructor now swaps the two (with a `debug_warn`) if they arrive inverted, before either is converted to cosine space, instead of producing a broken or divide-by-zero falloff.
- [x] **`WindowSpecification::isResizable` is dead config, never applied** — fixed: `Engine::Init` now calls `glfwWindowHint(GLFW_RESIZABLE, ...)` driven by this field, before window creation.
- [x] **`debug_info`/`debug_warn`/`debug_error` macros aren't wrapped in `do { ... } while(0)`** — fixed: all three, in both the Debug and Release (`NDEBUG`) branches, are now `do { ... } while (0)`-wrapped.
- [x] **`OrthoDemoScene::OnEvent`'s `KeyPressed` case has no `break`** — fixed: an explicit `break;` now closes the case before `default:`.
- [x] **`Text::Draw()` saves/restores the `GL_DEPTH_TEST`/`GL_BLEND`/`GL_CULL_FACE` enable bits but never the actual blend function or which texture was bound to unit 0 beforehand** — fixed: `Text::Draw()` now also saves/restores `GL_BLEND_SRC_RGB`/`GL_BLEND_DST_RGB`/`GL_ACTIVE_TEXTURE`/`GL_TEXTURE_BINDING_2D` around its draw, not just the three enable bits.
- [x] **`Font::Create` never validates `pixelHeight > 0`** — fixed: `if (pixelHeight <= 0.0f) { ...; return nullptr; }` guards the top of the function now.
- [x] **`Camera::SetUp()` calls `glm::normalize()` on its argument with no zero-vector guard** — fixed: now guards `glm::length(up) > 0.0f` before normalizing, silently no-oping on a zero vector instead of producing NaN.
- [x] **`LightDemoLayer`'s Q-key handler indexes `materials_[1]` directly instead of a tag lookup** — fixed: `RandomizeGold()` now looks up the Gold material by tag (`std::ranges::find_if` on `GetTag() == "Gold"`, with a not-found guard) instead of a magic index.
- [x] **`LoadShader`'s cache key could theoretically collide** — fixed: the cache key is now length-prefixed per path component (`size():path`) instead of a plain `"||"`-joined string — genuinely collision-proof, not just differently shaped, since each path's boundary is unambiguous regardless of its contents.
- [x] **Malformed `v`/`vn`/`vt` lines silently default missing components to `0.0`** — fixed: these lines now call `debug_warn(...)` on a missing-component default, matching the existing `f`-line token-count-check pattern.
- [x] **`SetOrthographic`/`SetBoost` accept invalid values with no validation** — fixed: `SetOrthographic` now guards `half_height <= 0.0f` and `SetBoost` guards `boost < 0.0f`, both warning and ignoring the call on an invalid value instead of accepting it.

---

## Known bugs / issues found in audit

Re-verified 2026-08-03 alongside the newer audit passes above: 11 of 12 entries below still
hold exactly as described (a couple with harmless line-number drift from later refactors,
not called out individually). Two corrections did come out of it, applied in place below —
entry 7's own "no `CLAUDE.md` exists" parenthetical is now false (one exists at the repo
root today), and entry 12's claimed fix (`0.1 *` ambient scale + `clamp()`) no longer exists
in `fragment.glsl` at all — the whole file was later rewritten into the multi-light
Blinn-Phong UBO system documented in Steps 6–7 below, which doesn't reuse that mechanism.

- [x] **Normal matrix computed on GPU** (`vertex.glsl:18`) — fixed: computed once per draw on the CPU (`glm::transpose(glm::inverse(modelMatrix))`) and uploaded as `uniform mat3 uNormalMatrix`; the vertex shader just does `vNormal = uNormalMatrix * aNormal` now. Originally landed in `TestLayer::OnRender`, later moved into base `Layer::Render()` — see the Completed section.
- [x] **OBJ loader drops non-triangle faces** (`Mesh.cpp:81`) — fixed: faces are parsed into a per-face vertex-index list first, then fan-triangulated (`indices[0]` anchors triangles against every consecutive pair) instead of requiring exactly 3 tokens. Assumes convex, planar faces — true for Blender's default export.
- [x] **`assert` in `UpdateRenderContext`** (`Scene.cpp`) — fixed: replaced with `if (cameras_.empty()) { debug_error(...); return; }`, so the check survives in release builds instead of compiling out.
- **`ResourceManager::LoadShader` cache ignores tag** (`ResourceManager.cpp:28`) — not a bug: tags aren't used to differentiate cache entries yet, this is intentional future-proofing until something actually needs per-tag shader variants.
- [x] **`getModelMatrix()` camelCase** (`Model.cpp:30`) — fixed: renamed to `GetModelMatrix()` everywhere (header, implementation, `TestLayer.cpp` call site).
- [x] **`if (!Init())` in Engine constructor doesn't actually guard construction** (`Engine.cpp:10`) — fixed: `debug_error` only throws when `NDEBUG` is unset, so in a release build it was a no-op and the constructor just `return`ed early, leaving the `Engine` half-constructed. The constructor now always `throw`s a plain `std::runtime_error` on `Init()` failure, independent of `NDEBUG` — `debug_error` still logs (and throws again, harmlessly unreachable) in debug builds, but the guard itself no longer depends on it.
- [x] **`Layer.hpp`'s interface doesn't match `CLAUDE.md`'s description** — resolved by decision, not by matching the doc to a pure-virtual `Destroy()`: none of `Suspend()`/`Transition()`/`Destroy()` need to be mandatory on every `Layer`. Suspend is just switching the active `Scene`/`Layer` (and now actually lives on `Scene`, not `Layer` — `Scene::Suspend()`, default no-op), `Destroy` is redundant with a subclass's own destructor running via `shared_ptr` (has a default `{}` body, not pure virtual), and `Transition` can be implemented however a given `Layer` needs (commented out of `Layer.hpp`) — none of it belongs on the base interface as a forced override. (Corrected note: this repo *does* have a `CLAUDE.md` at the repo root today — the original parenthetical claiming otherwise was stale as of the 2026-08-03 re-verification. The design decision itself is unaffected.)
- [x] **`Texture::Unbind()` unbinds whatever texture unit is currently active, not necessarily unit 0** (`Texture.cpp:59`) — fixed: `Unbind(GLuint slot = 0)` now calls `glActiveTexture(GL_TEXTURE0 + slot)` before unbinding.
- [x] **`LightType::Area` has no matching class** (`Light.hpp`) — resolved: enumerator dropped from `LightType` (never implemented, out of scope per `SCOPE.md`'s "area lights" exclusion).
- [x] **`Texture` never sets `GL_UNPACK_ALIGNMENT`** (`Texture.cpp`) — fixed: `glPixelStorei(GL_UNPACK_ALIGNMENT, 1);` added right before `glTexImage2D`.

Fixed in this audit:
- [x] **Back faces rendered** — `glEnable(GL_CULL_FACE)` + `glCullFace(GL_BACK)` added to `Engine::Init` (`Engine.cpp:58`)
- [x] **Fragment shader: no clamp, full ambient** — **stale, re-verified 2026-08-03 as no longer accurate.** The `0.1 *` ambient scale and output `clamp()` this entry describes do not exist in the current `fragment.glsl` — read the whole file to confirm: `main()`'s final line is `FragColor = vec4(totalLight, uBaseColor.a)` with no `clamp()` around it (the file's only `clamp()` call is unrelated, spotlight cone falloff math). This isn't a regression of a small fix so much as the fix's entire surrounding mechanism being gone — the single-light unlit-ish shader this was originally written against was later fully replaced by the multi-light Blinn-Phong UBO system (`Light`/`LightsBlock`, per-light-type functions, Steps 6–7 below). Not asserted as a live bug (the framebuffer's own write still clamps final output regardless of what the shader computes), just flagged so this entry stops implying a fix that isn't there — worth a fresh look only if stacked lights are ever observed blowing out to solid white.

---

## Open Architecture

- [x] **Rename `RenderContext`** — done: the struct and file are renamed to `FrameContext` (`include/Forge/FrameContext.hpp`, formerly `RenderContext.hpp`), every `Material::Bind`/`Layer::OnRender` call site takes `shared_ptr<FrameContext>`, and `Renderer::GetFrameContext()`/`Scene::UpdateFrameContext()` (formerly `GetRenderContext`/`UpdateRenderContext`) are renamed to match too.

- [x] **`ResourceManager::LoadMaterial`** — done: takes `(shared_ptr<Shader> shader, const string& tag, glm::vec3 ambient = ..., glm::vec3 diffuse = ..., glm::vec3 specular = ..., float shininess = ...)` with averaged defaults across the existing Blinn-Phong presets, caches by tag via the same weak_ptr pattern as `LoadMesh`/`LoadTexture`, and delegates to `Material::Create(shader, tag, ambient, diffuse, specular, shininess)`.

- [x] **Multiple cameras / split-screen** — done (Tier 3, see `SCOPE.md`): `Camera` owns a `Viewport` (normalized `[0,1]` rect + the window's current pixel size, recomputed on resize via `Camera::UpdateViewportSize`/`Viewport::SetWindowSize`, never a stale absolute-pixel rect). `Scene::Render()` loops over every camera in `cameras_` each frame — `ApplyViewport(i)` (`Viewport::Apply()`: `glViewport`+`glScissor`), `UpdateFrameContext(i)`, a scissored depth clear, `DrawBackground()`, then every layer's `Render()` — wrapped in `glEnable`/`glDisable(GL_SCISSOR_TEST)` so one camera's clear can't bleed into another's viewport. `active_camera_` stays purely an input-routing index — every camera in `cameras_` renders every frame regardless of which one is active. `MultiCameraDemoScene` exercises this: a free-fly main camera plus a fixed picture-in-picture overview (`Camera::SetPosition`/`SetYawPitch`, added alongside this).

- [x] **`Engine::HandleEvent`: app-settable global event hook** — done: `Engine::RaiseEvent` used to hardcode ESC-closes/Tab-cycles-scenes/backtick-toggles-cursor-capture directly in a switch statement, with no way for app code to see an event first or replace that policy. Now `RaiseEvent` runs fixed, always-on engine bookkeeping first (`WindowResize` → `FrameContext`/`glViewport`; `WindowLostFocus` → clears the held-key table, see below — neither gated behind the app handler's decision), then calls an app-settable `std::function<bool(Event&)>` (`Engine::SetEventHandler`, dispatched via `HandleEvent`) before forwarding to the active scene — same `false` = consumed/`true` = pass-through convention as `Layer::OnEvent`. New public methods (`CloseWindow`, `NextScene`/`PrevScene`, `SetCursorMode`/`GetCursorMode`) exist so a handler registered from `main.cpp` (composition, not subclassing — `Engine` is never meant to be subclassed) can actually replace what it's overriding, not just observe it. `main.cpp`'s handler is what implements ESC/Tab/backtick today; a different app could bind them differently, or not at all (e.g. ESC opening a menu instead of closing the window). Fixed the previously-known "Tab/backtick aren't actually intercepted" bug as a side effect (see Known Bugs above). `CursorMode` (`Captured`/`Normal`) replaces the old raw `GLint`/`GLFW_CURSOR_*` cursor-mode field, hand-transcribed to GLFW's real integer values the same way `Keys.hpp` does — no GLFW include needed.

- [x] **Held-key table + `WindowLostFocusEvent`** — done: `Engine::IsPressed(Key)`/`IsRepeat(Key)` read a `std::array<KeyState, 348>` kept live by `RaiseEvent` on every real `KeyPressedEvent`/`KeyReleasedEvent`, for input a single edge-triggered event can't express alone (e.g. checking a modifier is held from inside a different key's handler, for a shortcut like Alt+G). `KeyState::Released` is the enum's `0` value, so the zero-initialized array starts every key correctly not-held — an earlier version of this only initialized index 0 via brace-init and left every other key reading as permanently "pressed" until touched, caught before landing. `Key::Unknown` (`-1`) is explicitly skipped on both press and release, since casting it to an array index would wrap to a huge unsigned value and write out of bounds — also caught before landing (initially only guarded on the press path). `WindowLostFocusEvent` (new `EventType`, no payload) is raised by `EventHandler`'s `glfwSetWindowFocusCallback` only on the losing transition (an earlier version raised it on both directions, wiping the table every time focus was regained too — fixed); `RaiseEvent` clears the whole table on it, since GLFW never sends a `KeyReleased` for a key let go while the window was unfocused.

- [x] **Sub-mesh support** — done (Tier 3, see `SCOPE.md`): `Mesh::ParseObjFileGroups` buckets faces by whichever `usemtl` name was last seen (v/vn/vt parsing/indexing stays shared across the whole file, matching real OBJ semantics — only face-to-material association is grouped), returning one `ParsedMeshGroup` per distinct name in first-appearance order; per-group flat-normal generation runs independently so a `vn`-less multi-material file still gets correct per-face normals in every group. `usemtl` names are never resolved against a referenced `mtllib` — they're just a lookup key the caller maps to a real `Material` itself. `Mesh::CreateGroups(filename)` is the GPU-touching convenience wrapper (mirrors `Mesh::Create(filename)`), returning one `MeshGroup` (`materialName` + `shared_ptr<Mesh>`) per group. `Mesh::ParseObjFile`'s long-standing single-mesh contract is preserved exactly — it's now defined in terms of `ParseObjFileGroups`, merging multiple groups back into one combined vertex/index buffer (renumbered) when called directly, so existing callers/tests are unaffected. `Forge::Prop` (`include/Forge/Prop.hpp`) is the accompanying transform-sync piece: owns `vector<shared_ptr<Model>>` and forwards `SetPosition`/`SetScale`/`SetRotation` to every part, so a multi-`Model` object (e.g. the demo's `models/signpost.obj`, a post + board in two `usemtl` groups) moves as one rigid body without the caller hand-syncing N transforms — deliberately not a rendering concept, every part still registers into `Layer::materialModels_`/`materials_` exactly like any other `Model`. `LightDemoLayer`'s sixth station exercises both together.

- [x] **Click-to-pick / ray-casting primitives** — done (Tier 2, see `SCOPE.md`):
  `include/Forge/RayCast.hpp` (header-only, mirrors `Prop.hpp`'s shape) adds `Ray`
  (origin+direction) and `AABB` (min+max) as their own small types — not because either
  needs to be a type, but because both cross a producer/consumer boundary (a ray built in
  one place, tested in another) where two loose `vec3`s could get silently swapped —
  plus `IntersectRayAABB` (the slab method, `tMin` clamped to 0 so a box behind the ray's
  origin never counts as a hit), `IntersectRayPlane` (built for the click-and-drag-along-
  a-plane case Solitaire also needed; no caller yet), and `UnprojectScreenPoint` (screen
  pixel → NDC → `inverse(projection * view)` at both the near and far plane, dividing out
  `w` by hand since the GPU's own perspective divide doesn't happen for you going
  backward). `Mesh` gained a local-space AABB (`boundsMin`/`boundsMax`, computed once in
  `ComputeBounds()` from both GPU-mesh constructors, exposed via `GetLowerBounds()`/
  `GetUpperBounds()`); `Model::GetWorldBounds()` pushes that box's **8 corners** (not just
  the 2) through `GetModelMatrix()` and re-min/maxes them, since a rotated box's tightest
  axis-aligned fit isn't found from its min/max corners alone. `Camera::ScreenPointToRay()`
  is a thin wrapper pulling the camera's own `Viewport::GetPixelRect()` (new — `Apply()`
  now shares the same math instead of duplicating it) into `UnprojectScreenPoint`.
  `Layer::GetClickedObj(Ray)` walks the layer's own `materialModels_` (not the whole
  `Scene` — no broad-phase needed, ray/AABB is cheap enough to brute-force at TCG-scale
  object counts) and returns the closest hit as `(Material, Model)`; `Layer::GetClickedOnObj
  (MouseButtonPressedEvent&, FrameContext)` builds the ray for it — aiming from the actual
  cursor position in `Normal` cursor mode, or from the window's center pixel when
  `Captured` (FPS-style mouse-look reports a virtual, unbounded cursor position while
  disabled, not a real screen point — centering matches how a captured-cursor game like
  Minecraft always aims from its crosshair; unprojecting NDC `(0,0)` produces the same
  ray a dedicated "camera position + forward vector" path would, for any symmetric
  frustum). Needed `FrameContext::cursor_mode_` (mirrors `Engine`'s, kept in sync by
  `Engine::SetCursorMode`) and `MouseButtonEvent`/`MouseButtonPressedEvent`/
  `MouseButtonReleasedEvent` gaining `GetX()`/`GetY()` (GLFW's mouse-button callback
  doesn't hand you a position — `EventHandler.cpp` now calls `glfwGetCursorPos` itself).
  `Layer::OnEvent` changed signature to `(Event&, shared_ptr<FrameContext>)` to make any
  of this possible from inside a Layer at all (see the next bullet). Deliberately
  AABB-only, not OBB: a rotating object's tightest *axis-aligned* box necessarily grows
  and shrinks as it turns (up to ~1.73x at a cube's worst angle) rather than turning with
  it — that's the "AA" in AABB, not a bug. Accepted as fine for a TCG's mostly-flat,
  rarely-spinning cards; an OBB (tight at any angle, but a genuinely different, pricier
  intersection test — transform the ray into the box's local space first, or a full
  separating-axis test) is a "build it when something actually needs the tight fit"
  addition, not a gap to close now. `LightDemoLayer`'s left-click handler exercises the
  whole path: click a station,
  `GetClickedObj`'s hit material gets recolored, same as the `Q` handler but targeted.

- [x] **`Layer::OnEvent` gains `FrameContext`; `Layer` gains a name + show/hide** — done:
  `Layer::OnEvent(Event&, shared_ptr<FrameContext>)` is the `OnEvent`-half of the
  `SCOPE.md` Tier 2 "per-frame context in `OnEvent`/`OnUpdate`" item (`OnUpdate()` still
  doesn't receive one — that half is still open). Every `Scene::OnEvent` now forwards its
  own `fctx_` when dispatching to layers. Separately, `Layer` also gained a required
  constructor argument (`Layer(std::string name)` — every subclass must now pass one),
  `GetName()`, and `show_`/`Show()`/`Hide()`/`SetShow(bool)`/`IsShown()` — `Layer::Render()`
  now checks `show_` and skips a hidden layer's draw entirely (it didn't at first; the
  flag existed for one commit with no actual effect on rendering before this was wired
  in). `Scene::GetLayerByName(string)` does a by-name lookup (linear scan, matches
  `materials_`'s existing by-tag lookup pattern) — used by every demo scene's `F3` handler
  below. `Layer::GetAllModels()` flattens `materialModels_` into a plain list for anything
  outside a `Layer` that needs to see what it's drawing without reaching into its private
  buckets (its one caller so far is `DebugOverlayLayer`, next).

- [x] **`Forge::DebugOverlayLayer`** — done: a real engine-level debug-draw layer, not a
  demo-only hack (`include/Forge/DebugOverlayLayer.hpp`/`.cpp`), since there's no text/UI
  yet to build a numeric HUD with (`SCOPE.md` Tier 2) but a *visual* debug aid doesn't
  need text. Given a list of `Layer`s to watch, it snapshots every `Model` across them
  (via `GetAllModels()`, above) and draws a live wireframe box around each one's
  `Model::GetWorldBounds()` — the exact AABB click-to-pick relies on, finally visible
  instead of trusted blindly. One shared unit wireframe cube `Mesh` (`GL_LINES`, corners
  at ±0.5) is built once; `OnUpdate()` re-fits each tracked box's position/scale to its
  source model's current world AABB every frame via plain `SetPosition`/`SetScale` (no
  rotation — see the AABB-vs-OBB note in the click-to-pick bullet above for why a
  spinning object's box visibly grows/shrinks rather than turning with it). New minimal `shaders/debug_overlay_fragment.glsl`
  (pairs with the existing shared `vertex.glsl`) just outputs `uBaseColor`, unlit — proof
  that a `Material`/shader doesn't need to declare every uniform `Material::Bind()`
  uploads (`glGetUniformLocation` returning -1 for an unused name is a defined no-op, not
  an error). All three demo scenes construct one named `"Overlay"`, hidden by default,
  watching their `LightDemoLayer`, toggled by a new `F3` handler
  (`GetLayerByName("Overlay")` + `SetShow(!IsShown())`) in each scene's `OnEvent`.

---

## ✅ Text/UI Rendering — done

Verified end-to-end, not just compiled: the demo app was actually run (screenshot capture
against a live X session) and the "Randomize Gold" `Forge::Button` label renders correctly
— correctly positioned, alpha-blended against the sky gradient, no glyph culling. Full
GTest suite (97 tests, including the new `test_clickable_region.cpp`) passes. See the Known
Bugs entry above for the one real gap this surfaced (`Button` click routing under
`CursorMode::Captured`).

The design notes below are kept for rationale/context, not as an open plan:

- `Text` bypasses `Material`/`Layer::materialModels_` entirely. `Material::Bind()` always
  uploads the *active camera's* view/projection from `FrameContext`; text needs a fixed
  screen-space orthographic projection instead, independent of whatever 3D camera the
  scene is using. So `Text::Draw()` drives its own shader/texture/GL state directly —
  same pattern `Scene::DrawSkyboxBackground()`/`DrawSkydomeBackground()` already use for
  their own non-standard rendering needs.
- The text shader pair is deliberately minimal: vertex needs only `uProjection` (no
  `uModel`/`uView` — the pen position is already baked into each glyph quad's vertex data
  in pixel space); fragment needs only `uAtlas` + `uColor` (no `uMaterial`, no
  `uHasTexture`, no `uCameraPos`/`uTime`). Already written this way in
  `shaders/text_vertex.glsl`/`text_fragment.glsl` — don't add more uniforms there without
  a real reason to.

Now in place: `include/stb_truetype/stb_truetype.h` (vendored), `fonts/DejaVuSans.ttf`
+ `fonts/DejaVuSans-LICENSE.txt`, `shaders/text_vertex.glsl`/`text_fragment.glsl`,
`include/Forge/Rendering/Font.hpp`/`Text.hpp` + their `.cpp`s, `ResourceManager::LoadFont()`
implemented (weak_ptr-cached, keyed by `ttfPath + "@" + pixelHeight`).

### Step 1 — `Font.cpp` ✅

- [x] `#define STB_TRUETYPE_IMPLEMENTATION` + `#include "stb_truetype/stb_truetype.h"` in
      exactly this one file — same rule as `Texture.cpp`'s `STB_IMAGE_IMPLEMENTATION`.
- [x] `Font::Create(ttfPath, pixelHeight)`: reads the `.ttf` into a byte buffer, bakes a
      512x512 single-channel atlas via `stbtt_BakeFontBitmap`, uploads it as `GL_RED` with
      `GL_LINEAR`/`GL_CLAMP_TO_EDGE`. Private constructor via `std::shared_ptr<Font>(new Font())`.
- [x] `Font::~Font()`: `glDeleteTextures` if `atlasTexture_ != 0`.
- [x] `Font::GetGlyph(char c) const`: bounds-checked, falls back to `'?'` out of range.

### Step 2 — `Text.cpp` ✅

- [x] Constructor/`Destroy()`/`~Text()`: VAO/VBO/EBO lifecycle, same shape as `Mesh`'s own
      GPU resource ownership.
- [x] `Rebuild()`: walks `text_` char by char, mirrors `stbtt_GetBakedQuad`'s math (round
      the pen position, offset by `xoff`/`yoff`, size by `x1-x0`/`y1-y0`, UVs from
      `x0/atlasWidth` etc.), advances the pen by `xadvance`, uploads `GL_DYNAMIC_DRAW`.
      Tracks min/max corners for `GetSize()`. Only binds attribute locations 0/2.
- [x] `Draw()`: builds the y-down ortho projection, binds shader + atlas, draws. Both
      real bugs flagged below were hit for real building the reference implementation this
      was ported from and are handled here too: (1) `DebugOverlayLayer`'s text draws from
      `OnRender()`, registered after `LightDemoLayer` in every demo scene, so it isn't
      painted over. (2) `GL_CULL_FACE` (confirmed enabled globally in `Engine.cpp:58`) is
      disabled for the duration of `Draw()` and restored after, same save/restore as depth test.
- [x] `SetString()`/`SetPosition()` rebuild; `SetColor()` doesn't need to.

### Step 3 — `ResourceManager::LoadFont` ✅

- [x] Same weak_ptr caching shape as `LoadMesh`/`LoadShader`/`LoadTexture`, keyed by
      `ttfPath + "@" + std::to_string(pixelHeight)`.

### Step 4 — `CMakeLists.txt` ✅

- [x] `src/Forge/Rendering/Font.cpp`/`Text.cpp` added to `EngineCore`'s source list;
      `test/test_clickable_region.cpp` added to `EngineCore_tests`.

### Step 5 — wire a demo usage ✅

- [x] Proven end-to-end two ways: `DebugOverlayLayer`'s F3 overlay gained a live
      frame-time/FPS readout (`OnUpdate` now receives `FrameContext`, closing that Tier 2
      item too), and every demo scene got a real "Randomize Gold" `Forge::Button` via a new
      `Forge::UILayer`. Confirmed by actually running `OpenGL_App` and screenshotting it —
      the button's text renders correctly — not just a clean compile.

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

Empty — multi-camera/split-screen, Skybox/Skydome, and sub-mesh support, formerly listed
here, are all done. See Open Architecture and Completed above/below.

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
- **No picking primitives** — click-and-drag needs a screen-to-world ray (`Camera::ScreenPointToRay`, via `inverse(projection * view)`) and ray/AABB + ray/plane intersection tests. None of that existed; `Model` also needed a world-space bounding box (`GetWorldBounds()`) to intersect against. **Done** — see Open Architecture's "Click-to-pick / ray-casting primitives" entry. Ray/AABB and click-to-pick specifically are exercised (`LightDemoLayer`'s left-click handler); ray/plane exists but nothing calls it yet — click-and-*drag* itself still isn't built.
- **`Mesh` had no way to address a sub-rect of a texture** — needed a `CreateQuad(tag, uvMin, uvMax)` overload for texture-atlas sprites (52 playing cards sharing one atlas image, one quad per card pointing at its own cell).
- **`GL_BLEND` is never enabled anywhere in `Engine::Init`** — real alpha transparency (e.g. a translucent UI overlay/outline) silently does nothing until it's turned on. Costs nothing for existing fully-opaque textures, so there's no real downside to just always enabling it.
- **No animation/tweening utility** — ended up writing a small `Vec3Tween`/`EaseOutQuad` helper (lerp position A→B over a duration, with easing) from scratch for a card-slide animation. Small, generic, reusable — worth having in `Forge` rather than re-deriving per-project.
- **`Layer::OnEvent`/`OnUpdate` aren't given the camera/frame data (`FrameContext`)** — only `OnRender` gets it. Anything that needs view/projection for picking, or delta-time for animation, outside of `OnRender` has to cache whatever `OnRender` last saw. Works fine for a static camera; would break the moment the camera moves and an event fires between frames. **Half done** — `OnEvent` now takes a `shared_ptr<FrameContext>` (see Open Architecture); `OnUpdate()` still doesn't.
- **No text/UI rendering system** — this is why the current engine has no on-screen score/HUD/prompt of any kind, and Solitaire had to cut scoring/timer features and use a card "bounce" animation instead of a win banner. Worth keeping in mind as a real, currently-missing capability, not just a Solitaire-specific gap.
- **Orthographic cameras have a genuine invisible axis** — animating an object purely along the camera's own view direction is a real position change with *zero* visible pixels under orthographic projection (no perspective falloff to reveal it). Bit a "victory bounce" animation that moved along the wrong axis; only caught by actually looking at a rendered frame, not by checking the position math. Worth remembering for *any* animation added to an orthographic scene later.
- **Coincident flat quads z-fight** — two quads at the exact same depth (e.g. an animated sprite passing through a static marker at the same position) flicker unpredictably. Needed a small manual depth offset to fix; a proper convention (consistent tiny Z-bias for "decorative overlay" quads, or explicit draw-order control) would avoid needing to hand-fix this per case.
- **No way to screenshot/verify a render from outside the process** — every external screenshot tool failed in that sandbox. `glReadPixels` called directly from inside the render loop worked, but had to be re-added as throwaway temporary code every time a visual change needed checking, then removed again before committing. A small supported "step N frames headlessly and dump a framebuffer capture" hook on `Engine` would make this repeatable instead of hand-rolled each time — probably worth adding once this engine's demo scenes get complex enough that "did it compile" stops being enough to trust a change.

**Update — more lessons from a later Solitaire session**, adding undo/redo, a solver-backed "always deal a winnable game" guarantee, on-table icon buttons, and a couple of art/rendering fixes. These weren't visible in the first pass because that pass never resized the window, never added a second interactive UI element, and never needed real alpha edges on non-rectangular art:

- **Window resize is a silent trap the first pass never hit.** `Scene::ResizeCameras()` already exists (built for split-screen — recomputes every camera's aspect ratio from its viewport rect and the window's current size) but nothing calls it automatically; a `Scene` subclass has to remember to call it itself on `WindowResize`. Solitaire's scene never did. It compiled fine, ran fine, looked correct at whatever size it happened to boot at — and only visibly stretched every card the moment a user actually dragged a window corner. Two legitimate fixes exist depending on what a scene wants: auto-reflow (call `ResizeCameras()`) for something split-screen-like, or lock the window itself to a fixed ratio for a fixed-layout 2D scene (added `WindowSpecification::lockAspectRatio`, wired via `glfwSetWindowAspectRatio` in `Engine::Init`, opt-in and off by default). Worth considering whether `Scene`'s own resize handling should default to calling `ResizeCameras()` unless a subclass opts out, rather than requiring every subclass to remember to opt in — doing neither is the current silent default.
- **Still no reusable "clickable region" concept, even after building actual UI buttons.** Every hit-test in Solitaire — the stock pile, then an Undo button, then a Reset button — is the same hand-written "is this world point within half-width/half-height of this center" check, copy-pasted with new numbers each time. Sharpens the earlier "no text/UI system" lesson: the gap isn't just text rendering, it's that there's no lightweight "clickable sprite" or hit-region abstraction at all, so every new interactive element means re-deriving the same fixed-rect logic by hand. The underlying primitive this would be built on now exists (`Layer::GetClickedObj`/`GetClickedOnObj`, ray/AABB against `Model::GetWorldBounds()`) — but that's 3D-object picking, not a 2D UI-button/clickable-region abstraction; this bullet's actual ask is still open, and is `SCOPE.md` Tier 2's separate "clickable-region/UI-element abstraction" line item.
- **`GL_BLEND` being on doesn't mean transparency is fully safe — depth writes are a separate, still-open trap.** A blended fragment still writes its own depth to the depth buffer by default, alpha or no alpha, unless a draw explicitly disables depth writes for that pass. This didn't visibly break anything in Solitaire only by luck of geometry (new transparent card corners are small and rarely overlap another sprite that needed to show through at that exact pixel) — a scene with denser overlapping transparency (particles, layered UI) would hit real occlusion bugs from this. Distinct from the z-fight lesson above (that's two coincident depths flickering; this is occlusion ordering from a depth *write* that alpha doesn't suppress).
- **(Process, not code) Same-hue backgrounds can hide a broken alpha mask.** Solitaire's card atlas had solid green squares baked into its rounded corners, left over from the source art's own canvas color — unnoticed for a while because the table felt is *also* green, so a broken opaque corner and a correctly transparent one looked nearly identical in ad hoc testing. Worth a habit for any future art-import pipeline: preview a new alpha texture against a deliberately mismatched color before trusting it, not against the color it'll actually ship over.

---

## Completed

- **Test coverage expanded to 88 tests across 10 files** (from 54/6) — `test_tweens.cpp` (new, `Vec3Tween`'s Linear/EaseOutQuad interpolation, `IsDone`/clamping under `Repeat::None`, `Repeat::Loop`'s `fmod` wraparound, `Repeat::PingPong`'s direction reversal, `SetSpin`'s continuous-angle rotation representing a full turn that plain `slerp` can't, the finish callback firing once-per-lap rather than once-ever when repeating), `test_prop.cpp` (new, `Prop`'s transform-forwarding to every owned part, safe no-op on an empty part list), `test_viewport.cpp` (new, `Viewport`'s aspect-ratio math directly rather than only through `Camera`'s projection tests — deliberately skips `Apply()`, which needs a GL context, and the known 0×0 divide-by-zero bug), and three new `Camera::SetBoost` tests in `test_camera.cpp` (including a regression guard specifically for the strafe-boost bug fixed in the previous session). `Vec3Tween`/`Prop` tests use `Model(nullptr)` — the existing `test_model.cpp` trick — since neither class ever dereferences the mesh pointer, avoiding the need for a GL context.
- **Skybox/Skydome implemented** — `Scene::DrawSkyboxBackground()`/`DrawSkydomeBackground()` (previously `// TODO` stubs) are real now, both procedural (no textures/cubemaps). Skybox: a hardcoded 36-vertex unit cube (`Mesh::Create` from raw floats, no OBJ), `shaders/skybox_vertex.glsl`/`skybox_fragment.glsl` — vertex shader strips the view matrix's translation (`mat3(view)`) so the cube always surrounds the camera, and pins depth to the far plane (`gl_Position = vec4(pos.xy, pos.w, pos.w)`); fragment shader is a zenith/horizon/ground gradient plus a sun disc aimed opposite the scene's directional light. Skydome: no mesh at all — `shaders/skydome_vertex.glsl` builds a full-screen triangle purely from `gl_VertexID`, `skydome_fragment.glsl` reconstructs the view ray per-pixel via `inverse(projection*view)`, independently-tuned gradient+horizon haze. Both wrapped in `glDepthFunc(GL_LEQUAL)`/`glDepthMask(GL_FALSE)`; skybox additionally disables face culling for its one draw (camera is inside the cube). `Scene` gained a real destructor to release the skydome's raw VAO. New public `Scene::SetSkyboxBackground()`/`SetSkydomeBackground()` setters alongside the existing `SetBackgroundColor()`.
- **`Camera::SetPosition`/`SetYawPitch` added** — direct setters bypassing `CameraMove`/`Update`'s edge-triggered movement and `Rotate()`'s mouse-look path, for a fixed/scripted camera (e.g. a picture-in-picture overview) that never receives input.
- **`LightDemoScene`/`MultiCameraDemoScene` content fully redesigned** — `LightDemoLayer` now builds a floor (hardcoded quad `Vertex` data, no OBJ, same "raw vertex array" pattern as the skybox cube) plus five "stations" laid out along it: one cube per Blinn-Phong preset (Gold/Silver/Ruby/Emerald), each paired with a dedicated light in `OnSceneBoot` positioned/aimed directly at it so that light's effect dominates its own station instead of five lights blending uniformly over a scattered pile, plus a fifth station for the textured crate. `LightDemoScene` uses the new Skybox; `MultiCameraDemoScene` (same `LightDemoLayer` content, two cameras) uses the new Skydome, and its picture-in-picture camera is now a genuinely different elevated overview (`SetPosition`/`SetYawPitch`) instead of sharing the main camera's default pose.
- **`Engine::SetScene(GLint index)` + deferred scene switching** — bounds-checked, switches which registered scene is active; applied at the top of `Engine::Run()`'s loop via a `next_scene_` member (not immediately), so `current_scene_` can never change mid-frame between `Update()` and `RenderScene()`. `Tab` cycles scenes by default via `Engine::NextScene()` (`SetScene((current_scene_ + 1) % scenes_.size())`), called from `main.cpp`'s event handler since the `Engine::HandleEvent` rework (see Open Architecture) — originally this lived inline in `Engine::RaiseEvent` itself. This is why `LightDemoLayer`'s own randomize-color handler moved off `Tab` onto `Q`.
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
