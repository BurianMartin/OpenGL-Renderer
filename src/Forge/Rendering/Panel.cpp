#include "Forge/Rendering/Panel.hpp"
#include "Forge/Rendering/Mesh.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace
{
    /// Applied by `Panel::SetFillColor` to derive a subtle top-lighter/bottom-darker
    /// gradient pair from one flat color — see the class doc's "AutoShade" reference.
    constexpr float kAutoShadeFactor = 0.12f;
} // namespace

namespace Forge
{
    Panel::Panel(std::shared_ptr<Shader> shader, const glm::vec4 &fillColor, const glm::vec4 &borderColor,
                 float borderThicknessPx, float cornerRadiusPx)
        : shader_(std::move(shader)), borderColor_(borderColor), borderThicknessPx_(borderThicknessPx),
          cornerRadiusPx_(cornerRadiusPx)
    {
        SetFillColor(fillColor);
        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);
        glGenBuffers(1, &EBO_);
    }

    void Panel::SetFillColor(const glm::vec4 &color)
    {
        glm::vec3 rgb = glm::vec3(color);
        fillColorTop_ = glm::vec4(glm::clamp(rgb * (1.0f + kAutoShadeFactor), 0.0f, 1.0f), color.a);
        fillColorBottom_ = glm::vec4(glm::clamp(rgb * (1.0f - kAutoShadeFactor), 0.0f, 1.0f), color.a);
    }

    std::shared_ptr<Panel> Panel::Create(std::shared_ptr<ResourceManager> rmanager, float x, float y, float width,
                                          float height, const glm::vec4 &fillColor, const glm::vec4 &borderColor,
                                          float borderThicknessPx, float cornerRadiusPx)
    {
        auto shader = rmanager->LoadShader("shaders/panel_vertex.glsl", "shaders/panel_fragment.glsl", "Panel");
        if (!shader)
        {
            debug_warn("Panel::Create: missing shader");
            return nullptr;
        }

        auto panel = std::shared_ptr<Panel>(
            new Panel(std::move(shader), fillColor, borderColor, borderThicknessPx, cornerRadiusPx));
        panel->x_ = x;
        panel->y_ = y;
        panel->width_ = width;
        panel->height_ = height;
        panel->Rebuild();
        return panel;
    }

    void Panel::Destroy()
    {
        if (EBO_)
            glDeleteBuffers(1, &EBO_);
        if (VBO_)
            glDeleteBuffers(1, &VBO_);
        if (VAO_)
            glDeleteVertexArrays(1, &VAO_);
        EBO_ = VBO_ = VAO_ = 0;
    }

    Panel::~Panel()
    {
        Destroy();
    }

    void Panel::SetPosition(float x, float y)
    {
        x_ = x;
        y_ = y;
        Rebuild();
    }

    void Panel::SetSize(float width, float height)
    {
        width_ = width;
        height_ = height;
        Rebuild();
    }

    void Panel::SetRect(float x, float y, float width, float height)
    {
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
        Rebuild();
    }

    void Panel::Rebuild()
    {
        // vLocal (carried in the texCoords slot, same repurposing Text.cpp does for its own
        // UVs) runs 0..1 across the rect regardless of pixel size -- the fragment shader
        // turns borderThicknessPx_ into a fraction of width_/height_ at draw time instead,
        // since a Panel resize shouldn't need a geometry rebuild just to keep its border a
        // constant pixel width.
        Vertex vertices[4] = {
            {{x_, y_, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{x_ + width_, y_, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{x_ + width_, y_ + height_, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{x_, y_ + height_, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        };
        unsigned int indices[6] = {0, 1, 2, 0, 2, 3};

        glBindVertexArray(VAO_);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

        // Only aPos (0) and aTexCoords (2) -- same reasoning as Text::Rebuild, a Panel has
        // no use for a normal either.
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, position)));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, texCoords)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void Panel::Draw(GLint windowWidth, GLint windowHeight) const
    {
        if (width_ <= 0.0f || height_ <= 0.0f)
            return;

        GLboolean depthTestWasEnabled = glIsEnabled(GL_DEPTH_TEST);
        GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
        GLboolean cullFaceWasEnabled = glIsEnabled(GL_CULL_FACE);

        GLint prevBlendSrc, prevBlendDst;
        glGetIntegerv(GL_BLEND_SRC_RGB, &prevBlendSrc);
        glGetIntegerv(GL_BLEND_DST_RGB, &prevBlendDst);

        glDisable(GL_DEPTH_TEST); // a UI panel always draws on top of the 3D scene, regardless of draw order
        // Same y-down-ortho-flips-winding reasoning as Text::Draw -- without this the quad
        // gets silently back-face-culled.
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight), 0.0f, -1.0f, 1.0f);

        shader_->Use();
        shader_->SetMat4("uProjection", projection);
        shader_->SetVec4("uFillColorTop", fillColorTop_);
        shader_->SetVec4("uFillColorBottom", fillColorBottom_);
        shader_->SetVec4("uBorderColor", borderColor_);
        shader_->SetVec2("uSizePx", glm::vec2(width_, height_));
        shader_->SetFloat("uBorderThicknessPx", borderThicknessPx_);
        // Clamped so a radius request bigger than the Panel itself can't invert the SDF's
        // inner box (glm::min, not std::min -- width_/height_ are floats, no ambiguity, but
        // this matches the glm types already in play here).
        shader_->SetFloat("uCornerRadiusPx", glm::min(cornerRadiusPx_, 0.5f * glm::min(width_, height_)));

        shader_->SetBool("uHasTexture", texture_ != nullptr);
        if (texture_)
        {
            texture_->Bind(0);
            shader_->SetInt("uTexture", 0);
        }

        glBindVertexArray(VAO_);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        if (texture_)
            texture_->Unbind(0);

        glBlendFunc(static_cast<GLenum>(prevBlendSrc), static_cast<GLenum>(prevBlendDst));
        if (!blendWasEnabled)
            glDisable(GL_BLEND);
        if (depthTestWasEnabled)
            glEnable(GL_DEPTH_TEST);
        if (cullFaceWasEnabled)
            glEnable(GL_CULL_FACE);
    }
} // namespace Forge
