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
(`Application → Scene → Layer`, `ResourceManager`). v1.0 is that structure being *capable
and trustworthy enough that building the TCG on it is faster than starting from scratch*
— not matching SDL2's actual scope, which is a multi-decade, many-contributor project
this doesn't need to be.

## Definition of done — Framework v1.0

**Tier 1 — close the existing gap** (tracked in `ROADMAP.md`/Redline; Steps 6–7 done)
- [ ] Known-bug backlog (Redline Phase C — ~2.5–3.5 hrs)
- [ ] `ResourceManager::LoadMaterial` (Redline Phase D)
- [ ] Rename `RenderContext` → `FrameContext` (Redline Phase D)
- [x] Doxygen pass over `Core` — done, and extended to cover `App/` too (`TestLayer`/`TestScene`/`InputConfig`), which the original pass had explicitly left undocumented (see `ROADMAP.md`'s Completed section)

**Tier 2 — net-new, driven by exactly what the TCG needs, nothing more**
- [ ] **Text/UI rendering.** The most-flagged gap across every retrospective so far. A
  card game with no on-screen text for names/costs/life totals isn't shippable. The
  single biggest chunk of remaining work — see Redline Phase F.
- [ ] **A real clickable-region/UI-element abstraction.** Diagnosed via Solitaire;
  needed for any button or menu, not just cards. Redline Phase F.
- [ ] **`Layer` access to per-frame context in `OnEvent`/`OnUpdate`.** Diagnosed via
  Solitaire; a TCG's interactions (drag, targeting, hover previews) lean on this harder
  than a solitaire game did. Redline Phase F.
- [ ] **2D/orthographic camera mode + picking primitives.** Already built once for
  Solitaire, needs porting back into this engine. Redline Phase F.
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
    way Solitaire's `Game` namespace was — pure C++, zero dependency on `Core` or GL.
    That's what makes a **headless server build** possible: the same rules/state code,
    compiled into a separate server executable with no window, no GPU, no rendering at
    all, just network I/O and move validation. This doesn't exist yet in this engine —
    it's a pattern to repeat, not code to reuse — but it's proven to work by Solitaire's
    `Game`/`Core` split.

## Explicitly out of scope for v1.0 (maybe ever)

- ImGui / editor tooling — a separate project with its own UX design work, not a bolt-on
  feature.
- Matching SDL2's actual scope — multi-OS platform hardening, audio, gamepad support.
  GLFW already covers what this needs. **No audio.**
- 3D-only backlog items a flat card game doesn't need: skybox/skydome, sub-mesh support,
  area lights, multi-camera/split-screen. Left on `ROADMAP.md`/Redline as deferred, not
  deleted — revisit only if a future 3D project actually needs them, not for this one.
- Being a public, general-purpose library for other people — no public API contract, no
  semver, no back-compat promises. It only has to be good enough for this project and
  whatever comes after it.
- STUN/TURN, NAT traversal, peer discovery — solved by the home-server + VPN setup
  above, not by engine code.

## Path

1. Finish Tier 1 (mechanical, already estimated in Redline).
2. Build Tier 2, text/UI first — nothing else is even visible to a player without it.
3. Build the TCG's rules/state layer the way Solitaire's `Game` namespace was built:
   pure C++, zero `Core`/GL dependency. This unlocks the headless server for free.
4. Build the headless server + ENet client integration.
5. Build the TCG itself on top of all of the above — the integration test of the
   framework, the same role Solitaire played for the 2D/picking work. Expect it to
   surface things not visible yet, the way Solitaire did.
6. Ship it to friends, over the VPN, through the home server. That's the real finish
   line — not "the engine is done," but "people are playing, from separate computers."
