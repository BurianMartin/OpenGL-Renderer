#pragma once
#include "Utils.hpp"
#include <glad/gl.h>
#include <memory>
#include <string>

namespace Core
{
    class Texture
    {
    public:
        static std::shared_ptr<Texture> Create(const std::string &path);
        ~Texture();

        void Bind(GLuint slot = 0) const;
        void Unbind() const;

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
