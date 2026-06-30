#pragma once
#include "Utils.hpp"
#include "Core/Shader.hpp"
#include "Core/Texture.hpp"
#include "Core/RenderContext.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <string>

namespace Core
{
    class Material
    {
    public:
        static std::shared_ptr<Material> Create(std::shared_ptr<Shader> shader, const std::string &tag);
        ~Material() = default;

        void Bind(std::shared_ptr<RenderContext> ctx) const;

        void SetTexture(std::shared_ptr<Texture> texture);
        bool HasTexture() const;

        void SetColor(const glm::vec4 &color);
        void SetAmbient(const glm::vec3 &ambient);
        void SetDiffuse(const glm::vec3 &diffuse);
        void SetSpecular(const glm::vec3 &specular);
        void SetShininess(float shininess);

        std::string GetTag() const;

        std::shared_ptr<Shader> GetShader() const;

    private:
        Material(std::shared_ptr<Shader> shader, const std::string &tag);

        std::shared_ptr<Shader> shader_;
        std::shared_ptr<Texture> texture_;
        std::string tag_;

        glm::vec3 ambient_;
        glm::vec3 diffuse_;
        glm::vec3 specular_;
        glm::vec4 color_;
        float shininess_;
    };
} // namespace Core
