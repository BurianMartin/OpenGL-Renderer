# Scope

This file is the answer to "when is this engine done, and what is it for." `ROADMAP.md`
tracks the how; this tracks the how much and why. Read this before adding anything to
`ROADMAP.md`'s backlog — if it doesn't serve the goal below, it doesn't belong on v1.

## Ultimate goal

A digital version of the Riot TCG that friends can actually play **online**, built on an
engine general enough to be a real, reusable base for it — not a one-off hack, but not a
public product either. There is currently no usable alternative online, which is the
entire reason this is worth building.

## What "framework" means here

The SDL2 philosophy: a consumer defines their own `Scene`/`Layer` objects — game state,
rendering, input handling — and the framework owns the window/GL context, the render
loop, resource caching, and event dispatch. That structure already exists
(`Engine → Scene → Layer`, `ResourceManager`). v1.0 is that structure being *capable
and trustworthy enough that building the TCG on it is faster than starting from scratch*
— not matching SDL2's actual scope, which is a multi-decade, many-contributor project
this doesn't need to be.

## Definition of done — Framework v1.0

**Tier 1 — done** (tracked in `ROADMAP.md`/Redline; Steps 6–7 done)
- [x] Known-bug backlog (Redline Phase C — ~2.5–3.5 hrs)
- [x] `ResourceManager::LoadMaterial` (Redline Phase D)
- [x] Rename `RenderContext` → `FrameContext` (Redline Phase D) — struct, file, and the `Renderer::GetFrameContext()`/`Scene::UpdateFrameContext()` accessor names are all renamed
- [x] Doxygen pass over `Forge` — done, and extended to cover `App/` too (`TestLayer`/`TestScene`/`InputConfig`), which the original pass had explicitly left undocumented (see `ROADMAP.md`'s Completed section)

Tier 1 is closed — everything below is what's actually left for v1.0.

**Tier 2 — net-new, driven by exactly what the TCG needs, nothing more**
- [ ] **Text/UI rendering.** The most-flagged gap across every retrospective so far. A
  card game with no on-screen text for names/costs/life totals isn't shippable. The
  single biggest chunk of remaining work — see Redline Phase F. **In progress**:
  scaffolding is in (vendored `stb_truetype.h`, `fonts/DejaVuSans.ttf`, the text shader
  pair, `Font.hpp`/`Text.hpp` declarations, `ResourceManager::LoadFont` declared) but none
  of the actual logic is written yet — see `ROADMAP.md`'s "Next: Text/UI Rendering"
  section for the concrete build order.
- [ ] **A real clickable-region/UI-element abstraction.** Diagnosed via Solitaire;
  needed for any button or menu, not just cards. Redline Phase F.
- [ ] **`Layer` access to per-frame context in `OnUpdate`.** Diagnosed via Solitaire; a
  TCG's interactions (drag, targeting, hover previews) lean on this harder than a
  solitaire game did. Half done: `Layer::OnEvent` now takes a `shared_ptr<FrameContext>`
  (landed alongside the picking work below — it's what lets a click be turned into a
  world-space ray without the Layer needing its own Camera reference). `OnUpdate()` still
  doesn't receive one. Redline Phase F.
- [x] **2D/orthographic camera mode + picking primitives.** Orthographic mode was already
  done (`Camera::SetOrthographic`/`SetUp`, `OrthoDemoScene`); picking is done now too —
  `Forge::Ray`/`Forge::AABB` (`include/Forge/RayCast.hpp`), `UnprojectScreenPoint`,
  `IntersectRayAABB`, `Model::GetWorldBounds()`, `Camera::ScreenPointToRay()`, and
  `Layer::GetClickedObj()`/`GetClickedOnObj()` (walks a Layer's own `materialModels_`,
  no scene-wide broad-phase needed at this object count). `IntersectRayPlane` exists
  too but has no caller yet — it's there for the click-and-drag-along-a-plane case
  Solitaire also needed, not exercised by anything yet. Click-to-pick is intentionally
  AABB-only (not OBB) — a rotating object's axis-aligned box visibly grows/shrinks as it
  turns (see `ROADMAP.md`'s Open Architecture note); accepted as fine for a TCG's mostly-
  flat, rarely-spinning cards, revisit only if something actually needs a tight fit under
  rotation. The still-open half of the *original* bullet — a reusable clickable-region/UI
  abstraction (buttons, not just 3D objects) — stays its own separate bullet above.
- [ ] **Networking — client-server, this engine's home server as the always-on
  authoritative host.** Resolved design, not open:
  - Architecture: client-server, not peer-to-peer. One instance — the home server, not
    any player's machine — is authoritative. Turn-based gameplay means no rollback
    netcode, no client-side prediction, no interpolation is needed; the entire problem
    reduces to reliably delivering "player did action X" messages in order and applying
    the same deterministic mutation on every copy.
  - Transport: **ENet** (small, well-trodden reliable-UDP library for exactly this use
    case). A new dependency, but a lightweight, low-risk one — hand-rolled TCP framing
    isn't meaningfully simpler once reconnect edge cases are accounted for.
  - Connectivity: the home server has a public IP; friends connect over the existing
    home VPN. No STUN/TURN, no peer discovery, no NAT traversal code of any kind — the
    server's address is fixed and known.
  - Architecture consequence: the TCG's own rules/state layer must be written the same
    way Solitaire's `Game` namespace was — pure C++, zero dependency on `Forge` or GL.
    That's what makes a **headless server build** possible: the same rules/state code,
    compiled into a separate server executable with no window, no GPU, no rendering at
    all, just network I/O and move validation. This doesn't exist yet in this engine —
    it's a pattern to repeat, not code to reuse — but it's proven to work by Solitaire's
    `Game`/`Forge` split.

**Tier 3 — engine capability, not gated on the TCG at all.** Not required for v1.0 the
way Tier 2 is; these were revived as their own learning-exercise track independent of the
TCG critical path, after `SCOPE.md` had originally filed all three under "explicitly out
of scope." Kept as their own tier rather than silently folded into Tier 2 so that
distinction — genuinely useful engine work vs. TCG-required work — stays visible.
- [x] Multiple cameras / split-screen — done, see `ROADMAP.md`'s Open Architecture section
- [x] Skybox / Skydome — done, both implemented (procedural, no textures), see `ROADMAP.md`'s Completed section
- [x] Sub-mesh support — done, see `ROADMAP.md`'s Open Architecture section

All three Tier 3 items are now done.

## Explicitly out of scope for v1.0 (maybe ever)

- ImGui / editor tooling — a separate project with its own UX design work, not a bolt-on
  feature.
- Matching SDL2's actual scope — multi-OS platform hardening, audio, gamepad support.
  GLFW already covers what this needs. **No audio.**
- Area lights — never implemented, `LightType::Area` was dropped from the enum entirely
  (see `ROADMAP.md`'s Known Bugs). The only formerly-3D-only item genuinely staying out of
  scope — skybox/skydome, sub-mesh, and multi-camera moved to Tier 3 above instead.
- Being a public, general-purpose library for other people — no public API contract, no
  semver, no back-compat promises. It only has to be good enough for this project and
  whatever comes after it.
- STUN/TURN, NAT traversal, peer discovery — solved by the home-server + VPN setup
  above, not by engine code.

## Path

1. ~~Finish Tier 1 (mechanical, already estimated in Redline).~~ Done.
2. Build Tier 2, text/UI first — nothing else is even visible to a player without it.
3. Build the TCG's rules/state layer the way Solitaire's `Game` namespace was built:
   pure C++, zero `Forge`/GL dependency. This unlocks the headless server for free.
4. Build the headless server + ENet client integration.
5. Build the TCG itself on top of all of the above — the integration test of the
   framework, the same role Solitaire played for the 2D/picking work. Expect it to
   surface things not visible yet, the way Solitaire did.
6. Ship it to friends, over the VPN, through the home server. That's the real finish
   line — not "the engine is done," but "people are playing, from separate computers."
