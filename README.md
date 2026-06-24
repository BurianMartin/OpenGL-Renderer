# 🎮 OpenGL Renderer Core

![Language](https://img.shields.io/badge/language-C%2FC%2B%2B-blue)
![API](https://img.shields.io/badge/API-OpenGL-green)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey)
![Status](https://img.shields.io/badge/status-Work%20in%20Progress-orange)
![License](https://img.shields.io/badge/license-Open%20Source-purple)

# OpenGL Renderer Core

A lightweight, extensible OpenGL rendering framework designed to be 
the foundation for any OpenGL-based application or game.

## Vision

The goal is a clean core architecture where you bring your models and 
shaders — and the renderer handles the rest. Built around a layered 
scene system:

- **App** → contains **Scenes** → contain **Layers**
- Movement, rendering loop, and event handling managed by the core
- Drop in your assets and build on top

## Current Status
🚧 Work in progress — core architecture and basic rendering implemented.
Lighting and textures coming next.

## What's working
- Core renderer architecture
- Scene and layer system
- Basic geometry rendering
- Camera and movement

## Roadmap
- [ ] Texture support
- [ ] Lighting system
- [ ] Model loading
- [ ] Solitaire (first demo app built on the core)

## Tech
- C++
- OpenGL
- GLFW
- GLEW / GLAD

## Why open source?
This started as a solitaire game but evolved into something more useful — 
a reusable renderer anyone can build on top of.
