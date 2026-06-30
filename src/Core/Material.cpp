#include "Core/Material.hpp"

namespace Core
{
    std::shared_ptr<Material> Material::Create(std::shared_ptr<Shader> shader, const std::string &tag)
    {
        try
        {
            return std::shared_ptr<Material>(new Material(shader, tag));
        }
        catch (const std::exception &e)
        {
            debug_warn("Material creation failed: " << e.what());
            return nullptr;
        }
    }

    Material::Material(std::shared_ptr<Shader> shader, const std::string &tag)
        : shader_(shader), tag_(tag), ambient_(0.1f), diffuse_(1.0f), specular_(0.5f), color_(1.0f), shininess_(32.0f)
    {
    }

    void Material::Bind(std::shared_ptr<RenderContext> ctx) const
    {
        shader_->Use();

        shader_->SetMat4("uView", ctx->view_);
        shader_->SetMat4("uProjection", ctx->projection_);
        shader_->SetVec3("uCameraPos", ctx->camera_position_);
        shader_->SetFloat("uTime", ctx->time_);

        shader_->SetVec3("uMaterial.ambient", ambient_);
        shader_->SetVec3("uMaterial.diffuse", diffuse_);
        shader_->SetVec3("uMaterial.specular", specular_);
        shader_->SetFloat("uMaterial.shininess", shininess_);
        shader_->SetVec4("uBaseColor", color_);

        shader_->SetBool("uHasTexture", HasTexture());
        if (HasTexture())
        {
            texture_->Bind(0);
            shader_->SetInt("uTexture", 0);
        }
    }

    void Material::SetTexture(std::shared_ptr<Texture> texture) { texture_ = texture; }
    bool Material::HasTexture() const { return texture_ != nullptr; }

    void Material::SetColor(const glm::vec4 &color) { color_ = color; }
    void Material::SetAmbient(const glm::vec3 &ambient) { ambient_ = ambient; }
    void Material::SetDiffuse(const glm::vec3 &diffuse) { diffuse_ = diffuse; }
    void Material::SetSpecular(const glm::vec3 &specular) { specular_ = specular; }
    void Material::SetShininess(float shininess) { shininess_ = shininess; }

    std::string Material::GetTag() const { return tag_; }

    std::shared_ptr<Shader> Core::Material::GetShader() const
    {
        return shader_;
    }
} // namespace Core
