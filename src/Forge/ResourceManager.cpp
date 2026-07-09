#include "Forge/ResourceManager.hpp"
#include "ResourceManager.hpp"

namespace Forge
{

    std::shared_ptr<Mesh> ResourceManager::LoadMesh(const std::string &filename)
    {
        auto &weak = meshes_[filename];

        if (auto existing = weak.lock())
            return existing; // still alive

        try
        {
            auto fresh = Mesh::Create(filename); // load from disk
            weak = fresh;                        // cache weak ref
            return fresh;                        // scene gets owning ptr
        }
        catch (const std::exception &e)
        {
            debug_warn("Error loading mesh from filename: " + filename + " - " + e.what());
            return nullptr;
        }
    }

    std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string &vertex_filename, const std::string &fragment_filename, const std::string &tag)
    {
        auto &weak = shaders_[vertex_filename + "||" + fragment_filename];
        if (auto existing = weak.lock())
            return existing;
        auto fresh = Shader::Create(vertex_filename.c_str(), fragment_filename.c_str(), tag);
        weak = fresh;
        return fresh;
    }

    std::shared_ptr<Texture> ResourceManager::LoadTexture(const std::string &filename)
    {
        auto &weak = textures_[filename];
        if (auto existing = weak.lock())
            return existing;

        auto fresh = Texture::Create(filename);
        weak = fresh;
        return fresh;
    }

    std::shared_ptr<Material> ResourceManager::LoadMaterial(std::shared_ptr<Shader> shader, const std::string &tag, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess)
    {
        if (auto existing = materials_[tag].lock())
        {
            return existing;
        }
        auto material = Material::Create(shader, tag, ambient, diffuse, specular, shininess);
        materials_[tag] = material;
        return material;
    }

} // namespace Forge
