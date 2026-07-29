#include "Forge/Mesh.hpp"

namespace Forge
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

        ParsedMeshData parsed = ParseObjFile(file, filename);

        return std::shared_ptr<Mesh>(new Mesh(filename, parsed.vertices, parsed.indices, drawMode));
    }

    std::vector<MeshGroup> Mesh::CreateGroups(const std::string &filename, GLenum drawMode)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            debug_warn("Failed to open OBJ file: " << filename);
            return {};
        }

        std::vector<ParsedMeshGroup> parsedGroups;
        try
        {
            parsedGroups = ParseObjFileGroups(file, filename);
        }
        catch (const std::exception &e)
        {
            debug_warn("Failed to parse OBJ file groups: " << e.what());
            return {};
        }

        std::vector<MeshGroup> groups;
        groups.reserve(parsedGroups.size());
        for (auto &parsedGroup : parsedGroups)
        {
            auto mesh = Create(filename + "#" + parsedGroup.materialName, parsedGroup.mesh.vertices, parsedGroup.mesh.indices, drawMode);
            if (mesh)
                groups.push_back(MeshGroup{parsedGroup.materialName, mesh});
        }

        return groups;
    }

    ParsedMeshData Mesh::ParseObjFile(std::istream &input, const std::string &sourceName)
    {
        std::vector<ParsedMeshGroup> groups = ParseObjFileGroups(input, sourceName);

        if (groups.size() == 1)
            return std::move(groups[0].mesh);

        // More than one usemtl group but the caller only wants one mesh (this function's
        // contract predates group support): merge every group back into a single combined
        // vertex/index buffer, dropping material boundaries entirely.
        ParsedMeshData merged;
        for (auto &group : groups)
        {
            unsigned int base = static_cast<unsigned int>(merged.vertices.size());
            merged.vertices.insert(merged.vertices.end(), group.mesh.vertices.begin(), group.mesh.vertices.end());
            for (unsigned int index : group.mesh.indices)
                merged.indices.push_back(base + index);
        }
        return merged;
    }

    std::vector<ParsedMeshGroup> Mesh::ParseObjFileGroups(std::istream &input, const std::string &sourceName)
    {
        if (!std::filesystem::exists(sourceName))
        {
            debug_warn("File not found: " << sourceName);
        }

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;

        std::vector<Vertex> vertices;
        std::unordered_map<std::string, unsigned int> uniqueVertices;

        // Faces are bucketed by whichever `usemtl` name was most recently seen; faces
        // before the first `usemtl` line (or every face, in a file with none at all) land
        // in the default "" group. `groupOrder` preserves first-appearance order since
        // `unordered_map` doesn't.
        std::unordered_map<std::string, std::vector<unsigned int>> groupIndices;
        std::vector<std::string> groupOrder;
        std::string currentMaterial; // default "" group
        groupIndices[currentMaterial] = {};
        groupOrder.push_back(currentMaterial);

        std::string line;
        while (std::getline(input, line))
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
            else if (token == "usemtl")
            {
                ss >> currentMaterial;
                if (groupIndices.count(currentMaterial) == 0)
                {
                    groupIndices[currentMaterial] = {};
                    groupOrder.push_back(currentMaterial);
                }
            }
            else if (token == "f")
            {
                std::vector<std::string> faceTokens;
                std::string ft;
                while (ss >> ft)
                    faceTokens.push_back(ft);

                if (faceTokens.size() < 3)
                {
                    debug_warn("Degenerate face (< 3 vertices) encountered in " << sourceName << ", skipping");
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
                            debug_warn("OBJ vertex index out of range in " << sourceName);

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
                std::vector<unsigned int> &currentGroupIndices = groupIndices[currentMaterial];
                for (size_t i = 1; i + 1 < faceIndices.size(); ++i)
                {
                    currentGroupIndices.push_back(faceIndices[0]);
                    currentGroupIndices.push_back(faceIndices[i]);
                    currentGroupIndices.push_back(faceIndices[i + 1]);
                }
            }
        }

        if (vertices.empty())
            throw std::runtime_error("OBJ file produced no vertices: " + sourceName);

        bool needsFlatNormals = normals.empty();
        if (needsFlatNormals)
            debug_info("No normals found in " << sourceName << " — generating flat normals");

        std::vector<ParsedMeshGroup> result;
        for (const auto &materialName : groupOrder)
        {
            const std::vector<unsigned int> &sourceIndices = groupIndices[materialName];
            if (sourceIndices.empty())
                continue; // e.g. the default "" group when every face has a usemtl

            if (!needsFlatNormals)
            {
                result.push_back(ParsedMeshGroup{materialName, ParsedMeshData{vertices, sourceIndices}});
                continue;
            }

            // Expand to one vertex per index so each triangle gets its own unshared
            // vertices — shared vertices would get overwritten by the last face that
            // touches them, giving wrong normals at edges/corners. Done per group so
            // each group's ParsedMeshData stays self-contained.
            std::vector<Vertex> flat;
            std::vector<unsigned int> flatIndices;
            flat.reserve(sourceIndices.size());
            flatIndices.reserve(sourceIndices.size());

            for (size_t i = 0; i < sourceIndices.size(); i += 3)
            {
                Vertex v0 = vertices[sourceIndices[i]];
                Vertex v1 = vertices[sourceIndices[i + 1]];
                Vertex v2 = vertices[sourceIndices[i + 2]];

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

            result.push_back(ParsedMeshGroup{materialName, ParsedMeshData{std::move(flat), std::move(flatIndices)}});
        }

        debug_info("Loaded OBJ: " << sourceName << " — " << vertices.size() << " vertices, " << result.size() << " material group(s)");

        return result;
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
} // namespace Forge