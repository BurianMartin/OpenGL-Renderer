#pragma once
#include "Utils.hpp"

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>

namespace Core
{

    class Shader
    {
    public:
        std::string Tag_;

        GLuint ID;

        Shader(const char *vertexPath, const char *fragmentPath, const std::string &tag);
        ~Shader();

        void Use();

        void SetBool(const std::string &name, bool value);

        void SetInt(const std::string &name, int value);

        void SetFloat(const std::string &name, float value);

        void SetVec2(const std::string &name, const glm::vec2 &value);
        void SetVec3(const std::string &name, const glm::vec3 &value);
        void SetVec4(const std::string &name, const glm::vec4 &value);

        void SetMat2(const std::string &name, const glm::mat2 &value);
        void SetMat3(const std::string &name, const glm::mat3 &value);
        void SetMat4(const std::string &name, const glm::mat4 &value);

    private:
        void CheckCompileErrors(GLuint shader, const std::string &type);

        std::string LoadFile(const char *path);
    };
} // namespace Core
