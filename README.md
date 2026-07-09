# 🔨 Forge

**F**lexible **O**penGL **R**eal-time **G**raphics **E**ngine.

![Language](https://img.shields.io/badge/language-C%2FC%2B%2B-blue)
![API](https://img.shields.io/badge/API-OpenGL-green)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey)
![Status](https://img.shields.io/badge/status-Work%20in%20Progress-orange)
![License](https://img.shields.io/badge/license-Open%20Source-purple)

A C++20 OpenGL engine built around an `Engine → Scene → Layer` architecture: a
consumer defines their own `Scene`/`Layer` objects — game state, rendering, input
handling — and `Forge` owns the window/GL context, the render loop, resource
caching, and event dispatch.

## Why this exists

`Forge` is being built toward one concrete goal: a digital version of a trading
card game that friends can actually play online. It's not a general-purpose public
library — no public API contract, no semver, no back-compat promises — it only has
to be good enough for that project and whatever comes after it. See `SCOPE.md` for
the full reasoning, and `ROADMAP.md` for the open work tracked against it.

## Current status

The core rendering pipeline is done: scene/layer architecture, OBJ mesh loading,
Blinn-Phong lighting (directional/point/spot, multi-light UBO), diffuse + specular
texture mapping, a deduplicating resource cache, multi-camera/split-screen
rendering, and procedural Skybox/Skydome backgrounds. The known-bug backlog and
architecture cleanup (`Tier 1` in `SCOPE.md`) are closed out, and so is almost all
of `Tier 3` (engine-capability work, not gated on the TCG — only sub-mesh support
is left there). What's left is net-new, required work: text/UI rendering, a
clickable-region abstraction, **2D/orthographic camera support** (needed for any
top-down/UI-style view — genuinely necessary, not optional polish), and
client-server networking — see `SCOPE.md`'s Tier 2 for the full list.

## What's implemented

- `Engine → Scene → Layer` architecture, fully decoupled from demo-layer code
- Perspective camera with WASD + mouse-look
- Multi-camera/split-screen rendering — each `Camera` owns a normalized `Viewport` (see `Forge::Viewport`), independent of window resize
- Procedural `Skybox`/`Skydome` backgrounds (no textures/cubemaps)
- OBJ mesh loading (fan-triangulated n-gons, flat-normal generation when a model has no `vn` data)
- Blinn-Phong lighting via a `std140` UBO, with classic-OpenGL material presets (`Gold`, `Silver`, `Emerald`, ...)
- Diffuse + specular texture mapping
- `ResourceManager` — weak_ptr-cached loading for meshes, shaders, textures, and materials
- Two runnable demo scenes (`Demo` namespace, `make demo`) — `LightDemoScene` and `MultiCameraDemoScene`, both built on a shared `LightDemoLayer`
- Doxygen-generated API docs (`make doc`)
- A GTest suite covering the GL-independent logic (`make test`)

## Build

```bash
make          # configure + build
make run      # build then run from project root
make test     # build and run the test suite
make doc      # generate Doxygen docs
make demo     # build + run just the demo app
make lib      # build only libEngineCore.a, no demo code — for an external consumer project
```

See `CLAUDE.md` for the full build/test/architecture reference, `ROADMAP.md` for
open bugs and in-progress work, and `SCOPE.md` for why this exists and how much of
the roadmap is actually in scope.

## Tech

- C++20
- OpenGL (via glad)
- GLFW
- GLM
