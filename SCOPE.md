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
- [x] **Text/UI rendering.** The most-flagged gap across every retrospective so far. A
  card game with no on-screen text for names/costs/life totals isn't shippable. Done:
  `Forge::Font`/`Forge::Text` (stb_truetype baked-atlas rendering), verified end-to-end by
  actually running the demo app and screenshotting it, not just a clean compile — see
  `ROADMAP.md`'s "Text/UI Rendering" section.
- [x] **A real clickable-region/UI-element abstraction.** Diagnosed via Solitaire;
  needed for any button or menu, not just cards. Done: `Forge::ClickableRegion` (rect
  hit-test + callback), `Forge::Button` (`Text` + padded `ClickableRegion`), and
  `Forge::UILayer` (owns buttons/labels, routes clicks, registers last so it draws over
  and gets first look at input) — exercised by a working "Randomize Gold" button in every
  demo scene. `UILayer` correctly skips button hit-testing while `CursorMode::Captured`
  (see `ROADMAP.md`'s Known Bugs) — a captured cursor has no meaningful screen position to
  test a fixed 2D button against.
- [x] **`Layer` access to per-frame context in `OnUpdate`.** Diagnosed via Solitaire; a
  TCG's interactions (drag, targeting, hover previews) lean on this harder than a
  solitaire game did. Done: `Layer::OnUpdate(shared_ptr<FrameContext>)` now matches
  `OnEvent`'s signature — `DebugOverlayLayer`'s live frame-time/FPS readout is the first
  real user of it (previously had to live in `OnRender()` just to reach `delta_time_`).
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
- Matching SDL2's actual scope — multi-OS platform hardening, gamepad support. GLFW
  already covers what this needs.
- **Audio — reconsidered, not decided (2026-08-03).** Previously a hard "no audio" here
  alongside gamepad support; worth reopening now that the framework/library itself (not
  just the TCG) is the deliverable being shown off — a silent engine reads incomplete next
  to one that draws, lights, picks, and has UI. If taken on: a small non-positional 2D
  module (one-shot SFX + looping music, not full 3D positional audio) via a vendored
  single-header library (`miniaudio`), same vendoring pattern as `stb_image`/
  `stb_truetype`. Rough estimate ~3-4 hours: design (~30-45 min — SFX cached/fully-decoded
  vs. music streamed is a real API-shape decision, unlike Text/UI which had its design
  fully settled before implementation started), core `Sound`/`AudioClip`/`AudioEngine`
  classes (~1-1.5 hr), `ResourceManager`/`Engine` lifecycle wiring (~30-45 min), demo
  wiring + verification (~30-45 min). See "Immediate schedule" below for where this sits
  in the queue.
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
2. ~~Build Tier 2's text/UI rendering + clickable-region abstraction — nothing else is even visible to a player without it.~~ Done. Networking is the only Tier 2 item left.
3. Build the TCG's rules/state layer the way Solitaire's `Game` namespace was built:
   pure C++, zero `Forge`/GL dependency. This unlocks the headless server for free.
4. Build the headless server + ENet client integration.
5. Build the TCG itself on top of all of the above — the integration test of the
   framework, the same role Solitaire played for the 2D/picking work. Expect it to
   surface things not visible yet, the way Solitaire did.
6. Ship it to friends, over the VPN, through the home server. That's the real finish
   line — not "the engine is done," but "people are playing, from separate computers."

## Immediate schedule (as of 2026-08-03)

Day-by-day plan for right now, distinct from the longer-arc Path above:

1. **Bugs** — work through the 33 unfixed entries in `ROADMAP.md`'s Known Bugs section
   (full list re-verified against current code the same day it was logged; see that
   section's intro for what "verified" means here).
2. **Networking** — client-server via ENet, per the Tier 2 design above. Extensive
   back-and-forth expected — new territory, user-driven design discussion, not a handoff.
3. **Audio, maybe** — see the reconsidered "Audio" bullet above. Not decided yet; picked up
   only after bugs + networking, if at all.
