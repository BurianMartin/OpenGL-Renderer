#include "Forge/Core/ResourceManager.hpp"

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
        // Length-prefixed instead of a plain "||" join, so a path that happens to contain "||"
        // itself can never produce the same key as a different (vertex, fragment) pair.
        auto &weak = shaders_[std::to_string(vertex_filename.size()) + ":" + vertex_filename +
                               std::to_string(fragment_filename.size()) + ":" + fragment_filename];
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
        if (!shader)
        {
            debug_warn("Invalid shader provided to ResourceManager::LoadMaterial");
            return nullptr;
        }

        // Keyed by (shader, tag), not tag alone — otherwise two different shaders sharing a tag
        // string would silently collide and one would get served the other's shader.
        std::string key = std::to_string(reinterpret_cast<uintptr_t>(shader.get())) + ":" + tag;

        if (auto existing = materials_[key].lock())
        {
            return existing;
        }
        auto material = Material::Create(shader, tag, ambient, diffuse, specular, shininess);
        materials_[key] = material;
        return material;
    }

    std::shared_ptr<Font> ResourceManager::LoadFont(const std::string &ttfPath, float pixelHeight)
    {
        auto &weak = fonts_[ttfPath + "@" + std::to_string(pixelHeight)];
        if (auto existing = weak.lock())
            return existing;

        auto fresh = Font::Create(ttfPath, pixelHeight);
        weak = fresh;
        return fresh;
    }

} // namespace Forge
