#include "Mesh.hpp"

Core::Mesh::Mesh(const std::string &tag, const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, GLenum drawMode)
{
    this->tag = tag;
    setup(vertices, indices);
}

Core::Mesh::Mesh(const std::string &tag, const std::vector<GLfloat> &vertices, const std::vector<GLuint> &indices, GLenum drawMode)
{
    this->tag = tag;

    std::vector<Core::Vertex> vertexData;

    GLint vertexCount = vertices.size();

    for (int i = 0; i < vertexCount; i += 3)
    {
        Core::Vertex v;
        v.position = {vertices[i], vertices[i + 1], vertices[i + 2]};

        v.normal = {0.0f, 0.0f, 1.0f};
        v.texCoords = {0.0f, 0.0f};

        vertexData.push_back(v);
    }
    setup(vertexData, indices);
}

Core::Mesh::~Mesh()
{
}

void Core::Mesh::bind() const
{
    glBindVertexArray(VAO);
}

void Core::Mesh::unbind() const
{
    glBindVertexArray(0);
}

void Core::Mesh::draw() const
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

void Core::Mesh::setup(const std::vector<Vertex> &vertices,
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

void Core::Mesh::destroy()
{
}
