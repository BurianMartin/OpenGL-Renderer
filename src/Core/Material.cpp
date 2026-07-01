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

    static std::shared_ptr<Material> makePreset(std::shared_ptr<Shader> shader, const std::string &tag,
                                                glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess)
    {
        auto m = Material::Create(shader, tag);
        m->SetAmbient(ambient);
        m->SetDiffuse(diffuse);
        m->SetSpecular(specular);
        m->SetShininess(shininess);
        return m;
    }

    std::shared_ptr<Material> Material::Gold(std::shared_ptr<Shader> shader)
    {
        return makePreset(shader, "Gold",
                          {0.24725f, 0.1995f, 0.0745f}, {0.75164f, 0.60648f, 0.22648f}, {0.628281f, 0.555802f, 0.366065f}, 51.2f);
    }

    std::shared_ptr<Material> Material::Silver(std::shared_ptr<Shader> shader)
    {
        return makePreset(shader, "Silver",
                          {0.19225f, 0.19225f, 0.19225f}, {0.50754f, 0.50754f, 0.50754f}, {0.508273f, 0.508273f, 0.508273f}, 51.2f);
    }

    std::shared_ptr<Material> Material::Bronze(std::shared_ptr<Shader> shader)
    {
        return makePreset(shader, "Bronze",
                          {0.2125f, 0.1275f, 0.054f}, {0.714f, 0.4284f, 0.18144f}, {0.393548f, 0.271906f, 0.166721f}, 25.6f);
    }

    std::shared_ptr<Material> Material::Chrome(std::shared_ptr<Shader> shader)
    {
        return makePreset(shader, "Chrome",
                          {0.25f, 0.25f, 0.25f}, {0.4f, 0.4f, 0.4f}, {0.774597f, 0.774597f, 0.774597f}, 76.8f);
    }

    std::shared_ptr<Material> Material::Emerald(std::shared_ptr<Shader> shader)
    {
        return makePreset(shader, "Emerald",
                          {0.0215f, 0.1745f, 0.0215f}, {0.07568f, 0.61424f, 0.07568f}, {0.633f, 0.727811f, 0.633f}, 76.8f);
    }

    std::shared_ptr<Material> Material::Ruby(std::shared_ptr<Shader> shader)
    {
        return makePreset(shader, "Ruby",
                          {0.1745f, 0.01175f, 0.01175f}, {0.61424f, 0.04136f, 0.04136f}, {0.727811f, 0.626959f, 0.626959f}, 76.8f);
    }

    std::shared_ptr<Material> Material::Pearl(std::shared_ptr<Shader> shader)
    {
        return makePreset(shader, "Pearl",
                          {0.25f, 0.20725f, 0.20725f}, {1.0f, 0.829f, 0.829f}, {0.296648f, 0.296648f, 0.296648f}, 11.264f);
    }

    std::shared_ptr<Material> Material::Obsidian(std::shared_ptr<Shader> shader)
    {
        return makePreset(shader, "Obsidian",
                          {0.05375f, 0.05f, 0.06625f}, {0.18275f, 0.17f, 0.22525f}, {0.332741f, 0.328634f, 0.346435f}, 38.4f);
    }

    std::shared_ptr<Material> Material::Light(std::shared_ptr<Shader> shader)
    {
        return makePreset(shader, "Light",
                          {0.1f, 0.1f, 0.1f}, {0.8f, 0.8f, 0.8f}, {1.0f, 1.0f, 1.0f}, 128.0f);
    }

    std::shared_ptr<Material> Material::Copper(std::shared_ptr<Shader> shader)
    {
        return makePreset(shader, "Copper",
                          {0.19125f, 0.0735f, 0.0225f}, {0.7038f, 0.27048f, 0.0828f}, {0.256777f, 0.137622f, 0.086014f}, 12.8f);
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
