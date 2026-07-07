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

    std::shared_ptr<Mesh> Mesh::Create(const std::string &filename, GLenum drawMode)
    {
        std::ifstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("Failed to open OBJ file: " + filename);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::unordered_map<std::string, unsigned int> uniqueVertices;

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream ss(line);
            std::string token;
            ss >> token;

            if (token == "v")
            {
                glm::vec3 pos;
                ss >> pos.x >> pos.y >> pos.z;
                positions.push_back(pos);
            }
            else if (token == "vn")
            {
                glm::vec3 normal;
                ss >> normal.x >> normal.y >> normal.z;
                normals.push_back(normal);
            }
            else if (token == "vt")
            {
                glm::vec2 uv;
                ss >> uv.x >> uv.y;
                uv.y = 1.0f - uv.y; // flip Y for OpenGL
                texCoords.push_back(uv);
            }
            else if (token == "f")
            {
                std::vector<std::string> faceTokens;
                std::string ft;
                while (ss >> ft)
                    faceTokens.push_back(ft);

                if (faceTokens.size() < 3)
                {
                    debug_warn("Degenerate face (< 3 vertices) encountered in " << filename << ", skipping");
                    continue;
                }

                std::vector<unsigned int> faceIndices;
                faceIndices.reserve(faceTokens.size());

                for (const auto &ft : faceTokens)
                {
                    // deduplicate by raw token string e.g. "1/2/3"
                    if (uniqueVertices.count(ft) == 0)
                    {
                        uniqueVertices[ft] = static_cast<unsigned int>(vertices.size());

                        Vertex v{};
                        int vIdx = 0, vtIdx = -1, vnIdx = -1;

                        if (ft.find("//") != std::string::npos)
                        {
                            // v//vn
                            sscanf(ft.c_str(), "%d//%d", &vIdx, &vnIdx);
                        }
                        else
                        {
                            int matched = sscanf(ft.c_str(), "%d/%d/%d", &vIdx, &vtIdx, &vnIdx);
                            if (matched == 1)
                            { /* v only */
                            }
                            // matched == 2 → v/vt, matched == 3 → v/vt/vn
                        }

                        // OBJ is 1-based
                        if (vIdx > 0 && vIdx <= (int)positions.size())
                            v.position = positions[vIdx - 1];
                        else
                            debug_warn("OBJ vertex index out of range in " << filename);

                        if (vtIdx > 0 && vtIdx <= (int)texCoords.size())
                            v.texCoords = texCoords[vtIdx - 1];

                        if (vnIdx > 0 && vnIdx <= (int)normals.size())
                            v.normal = normals[vnIdx - 1];

                        vertices.push_back(v);
                    }

                    faceIndices.push_back(uniqueVertices[ft]);
                }

                // Fan-triangulate: assumes a convex, planar face (true for Blender's
                // default export). indices[0] anchors every triangle in the fan.
                for (size_t i = 1; i + 1 < faceIndices.size(); ++i)
                {
                    indices.push_back(faceIndices[0]);
                    indices.push_back(faceIndices[i]);
                    indices.push_back(faceIndices[i + 1]);
                }
            }
        }

        if (vertices.empty())
            throw std::runtime_error("OBJ file produced no vertices: " + filename);

        if (normals.empty())
        {
            debug_info("No normals found in " << filename << " — generating flat normals");

            // Expand to one vertex per index so each triangle gets its own
            // unshared vertices — shared vertices would get overwritten by the
            // last face that touches them, giving wrong normals at edges/corners.
            std::vector<Vertex> flat;
            std::vector<unsigned int> flatIndices;
            flat.reserve(indices.size());
            flatIndices.reserve(indices.size());

            for (size_t i = 0; i < indices.size(); i += 3)
            {
                Vertex v0 = vertices[indices[i]];
                Vertex v1 = vertices[indices[i + 1]];
                Vertex v2 = vertices[indices[i + 2]];

                glm::vec3 normal = glm::normalize(glm::cross(v1.position - v0.position, v2.position - v0.position));
                v0.normal = v1.normal = v2.normal = normal;

                unsigned int base = static_cast<unsigned int>(flat.size());
                flat.push_back(v0);
                flat.push_back(v1);
                flat.push_back(v2);
                flatIndices.push_back(base);
                flatIndices.push_back(base + 1);
                flatIndices.push_back(base + 2);
            }

            vertices = std::move(flat);
            indices  = std::move(flatIndices);
        }

        debug_info("Loaded OBJ: " << filename << " — " << vertices.size() << " vertices, " << indices.size() / 3 << " triangles");

        return std::shared_ptr<Mesh>(new Mesh(filename, vertices, indices, drawMode));
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
            v.position = {vertices[i], vertices[i + 1], vertices[i + 2]};
            v.normal = {0.0f, 0.0f, 0.0f};
            v.texCoords = {0.0f, 0.0f};
            vertexData.push_back(v);
        }

        // Accumulate face normals at each shared vertex (smooth shading)
        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            glm::vec3 p0 = vertexData[indices[i]].position;
            glm::vec3 p1 = vertexData[indices[i + 1]].position;
            glm::vec3 p2 = vertexData[indices[i + 2]].position;
            glm::vec3 n = glm::cross(p1 - p0, p2 - p0);
            vertexData[indices[i]].normal += n;
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

    Mesh::Mesh(Mesh &&other) noexcept
        : tag_(std::move(other.tag_)),
          VAO(other.VAO), VBO(other.VBO), EBO(other.EBO),
          vertexCount(other.vertexCount), indexCount(other.indexCount),
          drawMode(other.drawMode)
    {
        other.VAO = 0;
        other.VBO = 0;
        other.EBO = 0;
        other.vertexCount = 0;
        other.indexCount = 0;
    }

    Mesh &Mesh::operator=(Mesh &&other) noexcept
    {
        if (this != &other)
        {
            destroy();
            tag_ = std::move(other.tag_);
            VAO = other.VAO;
            VBO = other.VBO;
            EBO = other.EBO;
            vertexCount = other.vertexCount;
            indexCount = other.indexCount;
            drawMode = other.drawMode;
            other.VAO = 0;
            other.VBO = 0;
            other.EBO = 0;
            other.vertexCount = 0;
            other.indexCount = 0;
        }
        return *this;
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

        /*GLenum err = glGetError();
        if (err != GL_NO_ERROR)
            debug_error("GL error after draw: " << err);*/
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