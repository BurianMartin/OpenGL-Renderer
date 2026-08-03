#include "Forge/Rendering/Text.hpp"
#include "Forge/Rendering/Mesh.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace Forge
{
    Text::Text(std::shared_ptr<Font> font, std::shared_ptr<Shader> shader, const glm::vec4 &color)
        : font_(std::move(font)), shader_(std::move(shader)), color_(color)
    {
        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);
        glGenBuffers(1, &EBO_);
    }

    std::shared_ptr<Text> Text::Create(std::shared_ptr<ResourceManager> rmanager, std::shared_ptr<Font> font,
                                        const std::string &text, float x, float y, const glm::vec4 &color)
    {
        auto shader = rmanager->LoadShader("shaders/text_vertex.glsl", "shaders/text_fragment.glsl", "Text");
        if (!shader || !font)
        {
            debug_warn("Text::Create: missing shader or font");
            return nullptr;
        }

        auto text_obj = std::shared_ptr<Text>(new Text(std::move(font), std::move(shader), color));
        text_obj->text_ = text;
        text_obj->x_ = x;
        text_obj->y_ = y;
        text_obj->Rebuild();
        return text_obj;
    }

    void Text::Destroy()
    {
        if (EBO_)
            glDeleteBuffers(1, &EBO_);
        if (VBO_)
            glDeleteBuffers(1, &VBO_);
        if (VAO_)
            glDeleteVertexArrays(1, &VAO_);
        EBO_ = VBO_ = VAO_ = 0;
    }

    Text::~Text()
    {
        Destroy();
    }

    void Text::SetString(const std::string &text)
    {
        text_ = text;
        Rebuild();
    }

    void Text::SetPosition(float x, float y)
    {
        x_ = x;
        y_ = y;
        Rebuild();
    }

    void Text::SetColor(const glm::vec4 &color)
    {
        color_ = color;
    }

    void Text::Rebuild()
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        vertices.reserve(text_.size() * 4);
        indices.reserve(text_.size() * 6);

        float penX = x_;
        float penY = y_;
        glm::vec2 minCorner(std::numeric_limits<float>::max());
        glm::vec2 maxCorner(std::numeric_limits<float>::lowest());

        float atlasW = static_cast<float>(font_->GetAtlasWidth());
        float atlasH = static_cast<float>(font_->GetAtlasHeight());

        for (char c : text_)
        {
            if (c == '\n')
            {
                penX = x_;
                penY += font_->GetPixelHeight();
                continue;
            }

            const stbtt_bakedchar &glyph = font_->GetGlyph(c);

            // Mirrors stbtt_GetBakedQuad's own math (stb_truetype.h) rather than calling it
            // directly — that function writes into an stbtt_aligned_quad, which would just
            // mean copying its fields into a Vertex quad right after; building the Vertex
            // quad here skips that extra struct-to-struct step.
            int roundX = static_cast<int>(std::floor(penX + glyph.xoff + 0.5f));
            int roundY = static_cast<int>(std::floor(penY + glyph.yoff + 0.5f));

            float x0 = static_cast<float>(roundX);
            float y0 = static_cast<float>(roundY);
            float x1 = static_cast<float>(roundX + (glyph.x1 - glyph.x0));
            float y1 = static_cast<float>(roundY + (glyph.y1 - glyph.y0));

            float s0 = glyph.x0 / atlasW;
            float t0 = glyph.y0 / atlasH;
            float s1 = glyph.x1 / atlasW;
            float t1 = glyph.y1 / atlasH;

            unsigned int base = static_cast<unsigned int>(vertices.size());
            vertices.push_back({{x0, y0, 0.0f}, {0.0f, 0.0f, 1.0f}, {s0, t0}});
            vertices.push_back({{x1, y0, 0.0f}, {0.0f, 0.0f, 1.0f}, {s1, t0}});
            vertices.push_back({{x1, y1, 0.0f}, {0.0f, 0.0f, 1.0f}, {s1, t1}});
            vertices.push_back({{x0, y1, 0.0f}, {0.0f, 0.0f, 1.0f}, {s0, t1}});

            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);

            minCorner = glm::min(minCorner, glm::vec2(x0, y0));
            maxCorner = glm::max(maxCorner, glm::vec2(x1, y1));

            penX += glyph.xadvance;
        }

        size_ = vertices.empty() ? glm::vec2(0.0f) : (maxCorner - minCorner);
        indexCount_ = static_cast<unsigned int>(indices.size());

        glBindVertexArray(VAO_);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                     vertices.empty() ? nullptr : vertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                     indices.empty() ? nullptr : indices.data(), GL_DYNAMIC_DRAW);

        // Only aPos (0) and aTexCoords (2) — text has no use for a normal, so location 1
        // (which every other Mesh always binds) is simply left unset here.
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, position)));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, texCoords)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void Text::Draw(GLint windowWidth, GLint windowHeight) const
    {
        if (indexCount_ == 0)
            return;

        GLboolean depthTestWasEnabled = glIsEnabled(GL_DEPTH_TEST);
        GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
        GLboolean cullFaceWasEnabled = glIsEnabled(GL_CULL_FACE);

        glDisable(GL_DEPTH_TEST); // text always draws on top of the 3D scene, regardless of draw order
        // Engine::Init() enables back-face culling globally. The screen-space projection
        // below is built with a top-left origin/y-down convention (to match stb_truetype's
        // own baked-quad coordinates), which has a negative y-scale relative to standard
        // math/NDC orientation — that flip reverses each glyph quad's winding, so without
        // this it gets silently back-face-culled (GL_LINES, e.g. the wireframe boxes drawn
        // elsewhere in this layer, aren't affected by face culling at all, which is why
        // those were never a symptom of this). Same fix Scene::DrawSkyboxBackground()
        // already needed for its own inside-out-facing geometry.
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Screen-space pixel projection: (0,0) top-left, y grows downward — matches
        // stb_truetype's own baked-quad convention (see Rebuild() above), not the scene's
        // actual camera projection (Text bypasses Material::Bind() entirely — see class doc).
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight), 0.0f, -1.0f, 1.0f);

        shader_->Use();
        shader_->SetMat4("uProjection", projection);
        shader_->SetVec4("uColor", color_);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font_->GetAtlasTexture());
        shader_->SetInt("uAtlas", 0);

        glBindVertexArray(VAO_);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount_), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        if (!blendWasEnabled)
            glDisable(GL_BLEND);
        if (depthTestWasEnabled)
            glEnable(GL_DEPTH_TEST);
        if (cullFaceWasEnabled)
            glEnable(GL_CULL_FACE);
    }
} // namespace Forge
