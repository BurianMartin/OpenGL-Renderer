#pragma once

#include "Core/Mesh.hpp"
#include "Core/Shader.hpp"
#include "Core/Texture.hpp"
#include "Core/Material.hpp"

#include <memory>
#include <unordered_map>

namespace Core
{
    class ResourceManager
    {
    private:
        std::unordered_map<std::string, std::weak_ptr<Texture>> textures_;
        std::unordered_map<std::string, std::weak_ptr<Mesh>> meshes_;
        std::unordered_map<std::string, std::weak_ptr<Shader>> shaders_;
        std::unordered_map<std::string, std::weak_ptr<Material>> materials_;

    public:
        ResourceManager() = default;
        ~ResourceManager() = default;

        std::shared_ptr<Mesh> LoadMesh(const std::string &filename);
        std::shared_ptr<Shader> LoadShader(const std::string &vertex_filename, const std::string &fragment_filename, const std::string &tag);
        std::shared_ptr<Texture> LoadTexture(const std::string &filename);
        std::shared_ptr<Material> LoadMaterial(const std::string &tag);
    };
} // namespace Core
