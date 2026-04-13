#include "Model.hpp"

Model::Model(const float *vertices, size_t vertices_size, const unsigned int *indices, size_t indices_size, GLenum drw_mode)
{
    vertex_count = vertices_size / (3 * sizeof(float));
    index_count = indices ? indices_size / sizeof(unsigned int) : 0;
    draw_mode = drw_mode;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);

    if (indices && indices_size > 0)
    {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices, GL_STATIC_DRAW);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}

Model::~Model()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

Model::Model(Model &&other) noexcept
    : vao(other.vao), vbo(other.vbo), ebo(other.ebo),
      vertex_count(other.vertex_count), index_count(other.index_count),
      draw_mode(other.draw_mode), position(other.position),
      rotation(other.rotation), scale(other.scale)
{
    other.vao = 0;
    other.vbo = 0;
    other.ebo = 0;
}

Model &Model::operator=(Model &&other) noexcept
{
    if (this != &other)
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);

        vao = other.vao;
        vbo = other.vbo;
        ebo = other.ebo;
        vertex_count = other.vertex_count;
        index_count = other.index_count;
        draw_mode = other.draw_mode;
        position = other.position;
        rotation = other.rotation;
        scale = other.scale;

        other.vao = 0;
        other.vbo = 0;
        other.ebo = 0;
    }
    return *this;
}

glm::mat4 Model::GetModelMatrix()
{
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, position);
    m = glm::rotate(m, rotation.x, glm::vec3(1, 0, 0));
    m = glm::rotate(m, rotation.y, glm::vec3(0, 1, 0));
    m = glm::rotate(m, rotation.z, glm::vec3(0, 0, 1));
    m = glm::scale(m, scale);
    return m;
}

void Model::Draw()
{
    glBindVertexArray(vao);
    if (index_count > 0)
        glDrawElements(draw_mode, index_count, GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(draw_mode, 0, vertex_count);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        debug_error("GL error after draw: " << err);
}