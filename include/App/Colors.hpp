// Colors.h
#pragma once
#include <glm/glm.hpp>

namespace Solitare
{
    namespace Color_A1
    {
        const glm::vec4 Red = {1.0f, 0.0f, 0.0f, 1.0f};
        const glm::vec4 Green = {0.0f, 1.0f, 0.0f, 1.0f};
        const glm::vec4 Blue = {0.0f, 0.0f, 1.0f, 1.0f};

        const glm::vec4 White = {1.0f, 1.0f, 1.0f, 1.0f};
        const glm::vec4 Black = {0.0f, 0.0f, 0.0f, 1.0f};
        const glm::vec4 Gray = {0.5f, 0.5f, 0.5f, 1.0f};

        const glm::vec4 Yellow = {1.0f, 1.0f, 0.0f, 1.0f};
        const glm::vec4 Cyan = {0.0f, 1.0f, 1.0f, 1.0f};
        const glm::vec4 Magenta = {1.0f, 0.0f, 1.0f, 1.0f};

        const glm::vec4 Orange = {1.0f, 0.5f, 0.0f, 1.0f};
        const glm::vec4 Purple = {0.5f, 0.0f, 0.5f, 1.0f};
        const glm::vec4 Pink = {1.0f, 0.4f, 0.7f, 1.0f};

        const glm::vec4 Lime = {0.6f, 1.0f, 0.2f, 1.0f};
        const glm::vec4 SkyBlue = {0.4f, 0.7f, 1.0f, 1.0f};
        const glm::vec4 Brown = {0.6f, 0.3f, 0.1f, 1.0f};

        const glm::vec4 RandomColor();

    } // namespace Color

}