#include "Core/Mesh.hpp"

namespace Core
{
    std::shared_ptr<Mesh> Mesh::Create(const std::string &tag, const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, GLenum drawMode)
    {
        try
        {
            return std::shared_ptr<Mesh>(new Mesh(tag, vertices, indices, drawMode));
        }
        catch (const std::exception &e)
        {
            debug_warn("Mesh creation failed: " << e.what());
            return nullptr;
        }
    }

    std::shared_ptr<Mesh> Mesh::Create(const std::string &tag, const std::vector<GLfloat> &vertices, const std::vector<GLuint> &indices, GLenum drawMode)
    {
        try
        {
            return std::shared_ptr<Mesh>(new Mesh(tag, vertices, indices, drawMode));
        }
        catch (const std::exception &e)
        {
            debug_warn("Mesh creation failed: " << e.what());
            return nullptr;
        }
    }

    Mesh::Mesh(const std::string &tag, const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, GLenum drawMode)
        : drawMode(drawMode)
    {
        this->tag_ = tag;
        setup(vertices, indices);
    }

    Mesh::Mesh(const std::string &tag, const std::vector<GLfloat> &vertices, const std::vector<GLuint> &indices, GLenum drawMode)
        : drawMode(drawMode)
    {
        this->tag_ = tag;

        std::vector<Vertex> vertexData;
        vertexData.reserve(vertices.size() / 3);

        for (size_t i = 0; i + 2 < vertices.size(); i += 3)
        {
            Vertex v;
            v.position  = {vertices[i], vertices[i + 1], vertices[i + 2]};
            v.normal    = {0.0f, 0.0f, 0.0f};
            v.texCoords = {0.0f, 0.0f};
            vertexData.push_back(v);
        }

        // Accumulate face normals at each shared vertex (smooth shading)
        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            glm::vec3 p0 = vertexData[indices[i]].position;
            glm::vec3 p1 = vertexData[indices[i + 1]].position;
            glm::vec3 p2 = vertexData[indices[i + 2]].position;
            glm::vec3 n  = glm::cross(p1 - p0, p2 - p0);
            vertexData[indices[i]].normal     += n;
            vertexData[indices[i + 1]].normal += n;
            vertexData[indices[i + 2]].normal += n;
        }

        for (auto &v : vertexData)
        {
            float len = glm::length(v.normal);
            if (len > 1e-6f)
                v.normal /= len;
        }

        setup(vertexData, indices);
    }

    Mesh::~Mesh()
    {
        destroy();
    }

    void Mesh::bind() const
    {
        glBindVertexArray(VAO);
    }

    void Mesh::unbind() const
    {
        glBindVertexArray(0);
    }

    void Mesh::draw() const
    {
        bind();

        if (indexCount > 0)
            glDrawElements(drawMode, indexCount, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(drawMode, 0, vertexCount);

        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
            debug_error("GL error after draw: " << err);
    }

    void Mesh::setup(const std::vector<Vertex> &vertices,
                     const std::vector<unsigned int> &indices)
    {
        vertexCount = vertices.size();
        indexCount = indices.size();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
                     vertices.size() * sizeof(Vertex),
                     vertices.data(),
                     GL_STATIC_DRAW);

        // EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size() * sizeof(unsigned int),
                     indices.data(),
                     GL_STATIC_DRAW);

        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex),
                              (void *)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex),
                              (void *)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);

        // texCoords
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex),
                              (void *)offsetof(Vertex, texCoords));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void Mesh::destroy()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
} // namespace Core