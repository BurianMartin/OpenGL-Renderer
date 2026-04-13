#include "Shader.hpp"

class Model
{
public:
    // geometry
    GLuint vao;
    GLuint vbo;
    GLuint ebo; // optional, for indexed drawing
    GLint vertex_count;
    GLint index_count; // if using EBO

    GLenum draw_mode;

    // material
    GLuint texture; // later, when you get to textures

    // transform
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    glm::mat4 GetModelMatrix(); // builds the matrix from position/rotation/scale

public:
    Model(const float *vertices, size_t vertices_size,
          const unsigned int *indices, size_t indices_size, GLenum draw_mode);
    ~Model();

    Model(Model &&other) noexcept;
    Model &operator=(Model &&other) noexcept;

    void Draw(); // binds vao, calls glDrawArrays/glDrawElements
};