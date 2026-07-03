#pragma once
#include "Utils.hpp"

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <memory>
#include <sstream>
#include <unordered_map>

namespace Core
{

    /**
     * @brief Compiles/links a GLSL vertex+fragment program and exposes typed uniform setters.
     *
     * Construction is private — use `Create`, which compiles both stages
     * and links them, returning `nullptr` if a file is missing or
     * compilation/linking fails (the underlying constructor throws via
     * `debug_error`; `Create` catches it). Uniform locations are looked up
     * once per name and cached, so repeated `Set*` calls with the same name
     * are cheap.
     */
    class Shader
    {
    public:
        /**
         * @brief Loads, compiles, and links a shader program from two GLSL files.
         * @param vertexPath Path to the vertex shader source, relative to the working directory.
         * @param fragmentPath Path to the fragment shader source, relative to the working directory.
         * @param tag Identifier for later lookup (see GetTag()); has no effect on the GL program itself.
         * @return A new Shader, or `nullptr` if a file couldn't be loaded or compilation/linking failed.
         */
        static std::shared_ptr<Shader> Create(const char *vertexPath, const char *fragmentPath, const std::string &tag);

        ~Shader();

        /// Binds this program (`glUseProgram`) so subsequent `Set*`/draw calls apply to it.
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

        /// @return The tag this shader was created with.
        std::string GetTag() const;

    private:
        mutable std::unordered_map<std::string, GLint> uniform_cache_;

        Shader(const char *vertexPath, const char *fragmentPath, const std::string &tag);

        GLint GetUniformLocation(const std::string &name) const;
        void CheckCompileErrors(GLuint shader, const std::string &type);

        std::string LoadFile(const char *path);

        std::string Tag_;

        GLuint ID = 0;
    };
} // namespace Core
