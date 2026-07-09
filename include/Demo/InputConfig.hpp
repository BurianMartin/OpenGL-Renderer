#pragma once

#include <unordered_map>

#include "Utils.hpp"

#include "Forge/Camera.hpp"
#include "Forge/Keys.hpp"

/// Maps Forge::Key to Camera movement directions, used by LightDemoScene::OnEvent and
/// MultiCameraDemoScene::OnEvent to translate raw WASD/Space/Left-Shift key events into
/// `Camera::CameraMove` calls.
inline const std::unordered_map<Forge::Key, Forge::CamMove> key_map = {
    {Forge::Key::W, Forge::CamMove::FORWARD},
    {Forge::Key::S, Forge::CamMove::BACKWARD},
    {Forge::Key::A, Forge::CamMove::LEFT},
    {Forge::Key::D, Forge::CamMove::RIGHT},
    {Forge::Key::Space, Forge::CamMove::UP},
    {Forge::Key::LeftShift, Forge::CamMove::DOWN}};