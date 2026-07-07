#pragma once
#include "Utils.hpp"
#include <glad/gl.h>
#include <memory>
#include <string>

namespace Core
{
    /**
     * @brief Owns one GPU 2D texture, loaded from an image file via stb_image.
     *
     * Construction is private — use `Create`. RGB vs RGBA is detected from
     * the source image's channel count. The destructor releases the GPU
     * texture (`glDeleteTextures`).
     */
    class Texture
    {
    public:
        /**
         * @brief Loads an image from disk and uploads it as a GPU texture.
         * @param path Path to the image file, relative to the working directory. Loaded with a vertical flip (stb_image `stbi_set_flip_vertically_on_load`) to match OpenGL's bottom-left texture origin.
         */
        static std::shared_ptr<Texture> Create(const std::string &path);
        ~Texture();

        /// Binds this texture to a texture unit (`glActiveTexture(GL_TEXTURE0 + slot)` then `glBindTexture`); pair with `Shader::SetInt` to point a sampler uniform at the same slot.
        void Bind(GLuint slot = 0) const;

        /// Unbinds `GL_TEXTURE_2D` on whichever texture unit is currently active (i.e. whatever `glActiveTexture` was last set to — typically the slot passed to the most recent Bind() call, not necessarily unit 0).
        void Unbind(GLuint slot = 0) const;

        /// @return The path this texture was loaded from.
        std::string GetPath() const;
        int GetWidth() const;
        int GetHeight() const;

    private:
        Texture(const std::string &path);

        GLuint ID_ = 0;
        int width_ = 0;
        int height_ = 0;
        std::string path_;
    };
} // namespace Core
