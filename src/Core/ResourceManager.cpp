#include "Core/ResourceManager.hpp"

namespace Core
{

    std::shared_ptr<Mesh> ResourceManager::LoadMesh(const std::string &filename)
    {
        auto &weak = meshes_[filename];

        if (auto existing = weak.lock())
            return existing;                 // still alive
        auto fresh = Mesh::Create(filename); // load from disk
        weak = fresh;                        // cache weak ref
        return fresh;                        // scene gets owning ptr
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

} // namespace Core
