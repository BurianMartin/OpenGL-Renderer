#include "Shader.hpp"

namespace Core
{
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

    void Shader::SetBool(const std::string &name, bool value)
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    void Shader::SetInt(const std::string &name, int value)
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void Shader::SetFloat(const std::string &name, float value)
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void Shader::SetVec2(const std::string &name, const glm::vec2 &value)
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }

    void Shader::SetVec3(const std::string &name, const glm::vec3 &value)
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }

    void Shader::SetVec4(const std::string &name, const glm::vec4 &value)
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }

    void Shader::SetMat2(const std::string &name, const glm::mat2 &value)
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::SetMat3(const std::string &name, const glm::mat3 &value)
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::SetMat4(const std::string &name, const glm::mat4 &value)
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::CheckCompileErrors(GLuint shader, const std::string &type)
    {
        int success;
        char infoLog[512];

        if (type == "PROGRAM")
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 512, NULL, infoLog);
                debug_warn("Program linking error: " << infoLog);
            }
        }
        else
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 512, NULL, infoLog);
                debug_warn("Shader compile error (" << type << "): " << infoLog);
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
