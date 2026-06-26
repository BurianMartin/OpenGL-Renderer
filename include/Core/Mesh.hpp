#pragma once
#include "Utils.hpp"

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace Core
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
        // glm::vec4 color;  -- add if you need per-vertex color
    };

    class Mesh
    {
    public:
        // --- Factory ---
        static std::shared_ptr<Mesh> Create(const std::string &tag, const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, GLenum drawMode = GL_TRIANGLES);
        static std::shared_ptr<Mesh> Create(const std::string &tag, const std::vector<GLfloat> &vertices, const std::vector<GLuint> &indices, GLenum drawMode = GL_TRIANGLES);
        static std::shared_ptr<Mesh> Create(const std::string &filename, GLenum drawMode = GL_TRIANGLES);

        ~Mesh();

        // Non-copyable (owns GPU resources)
        Mesh(const Mesh &) = delete;
        Mesh &operator=(const Mesh &) = delete;

        // Movable
        Mesh(Mesh &&other) noexcept;
        Mesh &operator=(Mesh &&other) noexcept;

        // --- Rendering ---
        void bind() const;
        void unbind() const;
        void draw() const; // binds, draws

        // --- Accessors ---
        unsigned int getVertexCount() const { return vertexCount; }
        unsigned int getTriangleCount() const { return indexCount / 3; }
        GLenum getDrawMode() const { return drawMode; }

    private:
        Mesh(const std::string &tag, const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, GLenum drawMode);
        Mesh(const std::string &tag, const std::vector<GLfloat> &vertices, const std::vector<GLuint> &indices, GLenum drawMode);

        void setup(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);
        void destroy();

        std::string tag_;

        GLuint VAO = 0;
        GLuint VBO = 0;
        GLuint EBO = 0;

        unsigned int vertexCount = 0;
        unsigned int indexCount = 0;
        GLenum drawMode = GL_TRIANGLES;
    };
}
