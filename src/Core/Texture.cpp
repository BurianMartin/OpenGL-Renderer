#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "Core/Texture.hpp"

namespace Core
{
    std::shared_ptr<Texture> Texture::Create(const std::string &path)
    {
        try
        {
            return std::shared_ptr<Texture>(new Texture(path));
        }
        catch (const std::exception &e)
        {
            debug_warn("Texture creation failed: " << e.what());
            return nullptr;
        }
    }

    Texture::Texture(const std::string &path) : path_(path)
    {
        stbi_set_flip_vertically_on_load(true);

        int nrChannels;
        unsigned char *data = stbi_load(path.c_str(), &width_, &height_, &nrChannels, 0);

        if (!data)
            debug_error("Failed to load texture: " + path);

        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glGenTextures(1, &ID_);
        glBindTexture(GL_TEXTURE_2D, ID_);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width_, height_, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &ID_);
    }

    void Texture::Bind(GLuint slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, ID_);
    }

    void Texture::Unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    std::string Texture::GetPath() const { return path_; }
    int Texture::GetWidth() const { return width_; }
    int Texture::GetHeight() const { return height_; }
} // namespace Core
