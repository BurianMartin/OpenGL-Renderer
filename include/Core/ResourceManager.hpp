#pragma once

#include "Core/Mesh.hpp"
#include "Core/Shader.hpp"
#include "Core/Texture.hpp"
#include "Core/Material.hpp"

#include <memory>
#include <unordered_map>

namespace Core
{
    /**
     * @brief Deduplicating loader/cache for GPU assets (meshes, shaders, textures, materials).
     *
     * Shared between Application, Scene, and Layer. Each `Load*` call caches
     * its result by key via a `weak_ptr`: if the returned shared_ptr from a
     * previous load is still alive, the cached object is returned instead
     * of reloading from disk; once every owner releases it, the next
     * `Load*` call reloads fresh.
     */
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

        /// @param filename Path to an `.obj` file, cached by filename. @return The mesh, or `nullptr` if loading failed.
        std::shared_ptr<Mesh> LoadMesh(const std::string &filename);

        /**
         * @brief Loads (or returns the cached) shader for a vertex+fragment pair.
         * @param vertex_filename Path to the vertex shader source; part of the cache key.
         * @param fragment_filename Path to the fragment shader source; part of the cache key.
         * @param tag Forwarded to `Shader::Create` for the new shader, but @warning not part of the cache key — the cache key is only `vertex_filename + "||" + fragment_filename`, so calling this twice with the same paths but different tags returns the first cached shader with its original tag, not a new one.
         */
        std::shared_ptr<Shader> LoadShader(const std::string &vertex_filename, const std::string &fragment_filename, const std::string &tag);

        /// @param filename Path to an image file, cached by filename. @return The texture, or `nullptr` if loading failed.
        std::shared_ptr<Texture> LoadTexture(const std::string &filename);

        /// @warning Not yet implemented.
        std::shared_ptr<Material> LoadMaterial(const std::string &tag);
    };
} // namespace Core
