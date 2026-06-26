#include "Shader.hpp"

namespace Core
{
    std::shared_ptr<Shader> Shader::Create(const char *vertexPath, const char *fragmentPath, const std::string &tag)
    {
        try
        {
            return std::shared_ptr<Shader>(new Shader(vertexPath, fragmentPath, tag));
        }
        catch (const std::exception &e)
        {
            debug_warn("Shader creation failed: " << e.what());
            return nullptr;
        }
    }

    Shader::Shader(const char *vertexPath, const char *fragmentPath, const std::string &tag)
        : Tag_(tag)
    {
        std::string vertSrc = LoadFile(vertexPath);
        std::string fragSrc = LoadFile(fragmentPath);

        const char *vCode = vertSrc.c_str();
        const char *fCode = fragSrc.c_str();

        GLuint vertexShader;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vCode, NULL);
        glCompileShader(vertexShader);
        CheckCompileErrors(vertexShader, "VERTEX");

        GLuint fragmentShader;
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fCode, NULL);
        glCompileShader(fragmentShader);
        CheckCompileErrors(fragmentShader, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        glLinkProgram(ID);
        CheckCompileErrors(ID, "PROGRAM");

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        debug_info("Shader program ID: " << ID);
    }

    void Shader::Use()
    {
        glUseProgram(ID);
    }

    Shader::~Shader()
    {
        glDeleteProgram(ID);
    }

    GLint Shader::GetUniformLocation(const std::string &name) const
    {
        auto it = uniform_cache_.find(name);
        if (it != uniform_cache_.end())
            return it->second;

        GLint location = glGetUniformLocation(ID, name.c_str());
        uniform_cache_[name] = location;
        return location;
    }

    void Shader::SetBool(const std::string &name, bool value)
    {
        glUniform1i(GetUniformLocation(name), (int)value);
    }

    void Shader::SetInt(const std::string &name, int value)
    {
        glUniform1i(GetUniformLocation(name), value);
    }

    void Shader::SetFloat(const std::string &name, float value)
    {
        glUniform1f(GetUniformLocation(name), value);
    }

    void Shader::SetVec2(const std::string &name, const glm::vec2 &value)
    {
        glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
    }

    void Shader::SetVec3(const std::string &name, const glm::vec3 &value)
    {
        glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
    }

    void Shader::SetVec4(const std::string &name, const glm::vec4 &value)
    {
        glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
    }

    void Shader::SetMat2(const std::string &name, const glm::mat2 &value)
    {
        glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::SetMat3(const std::string &name, const glm::mat3 &value)
    {
        glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::SetMat4(const std::string &name, const glm::mat4 &value)
    {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    std::string Shader::GetTag() const
    {
        return Tag_;
    }

    void Shader::CheckCompileErrors(GLuint shader, const std::string &type)
    {
        int success;

        if (type == "PROGRAM")
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                GLint len = 0;
                glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &len);
                std::string infoLog(len, '\0');
                glGetProgramInfoLog(shader, len, nullptr, infoLog.data());
                debug_error("Program linking error: " << infoLog);
            }
        }
        else
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                GLint len = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
                std::string infoLog(len, '\0');
                glGetShaderInfoLog(shader, len, nullptr, infoLog.data());
                debug_error("Shader compile error (" << type << "): " << infoLog);
            }
        }
    }

    std::string Shader::LoadFile(const char *path)
    {
        std::ifstream file;
        std::stringstream stream;

        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            file.open(path);
            stream << file.rdbuf();
            file.close();
        }
        catch (std::ifstream::failure &e)
        {
            debug_error("Shader file not found: " + std::string(path));
        }

        return stream.str();
    }

} // namespace Core
