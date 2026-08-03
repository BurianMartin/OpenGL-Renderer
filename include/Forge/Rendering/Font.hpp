#pragma once
#include "Utils.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "stb_truetype/stb_truetype.h"

#include <array>
#include <memory>
#include <string>

namespace Forge
{
    /**
     * @brief A baked bitmap font atlas: one GPU texture holding every glyph's rasterized
     * bitmap, plus per-glyph metrics (stb_truetype's `stbtt_bakedchar`) needed to lay text out.
     *
     * Construction is private — use `Create()`, which returns `nullptr` instead of throwing
     * if the file couldn't be read or the baked glyphs didn't fit the atlas. Only printable
     * ASCII (32..127, space through `~`) is baked — no Unicode/i18n support, not needed yet.
     */
    class Font
    {
    public:
        static constexpr int kFirstChar = 32;
        static constexpr int kNumChars = 96; // [32, 128)

        /**
         * @param ttfPath Path to a `.ttf` file, resolved relative to the process's working directory.
         * @param pixelHeight Font size, in pixels, to bake the atlas at. Text drawn at a very
         * different on-screen size will look soft or blocky — bake close to actual use.
         * @return A new Font, or `nullptr` on failure.
         */
        static std::shared_ptr<Font> Create(const std::string &ttfPath, float pixelHeight);

        ~Font();
        Font(const Font &) = delete;
        Font &operator=(const Font &) = delete;

        /// @return Baked metrics for `c`, or for `'?'` if `c` falls outside the baked [32, 128) range.
        const stbtt_bakedchar &GetGlyph(char c) const;

        GLuint GetAtlasTexture() const { return atlasTexture_; }
        int GetAtlasWidth() const { return atlasWidth_; }
        int GetAtlasHeight() const { return atlasHeight_; }
        float GetPixelHeight() const { return pixelHeight_; }

    private:
        Font() = default;

        GLuint atlasTexture_ = 0;
        int atlasWidth_ = 0;
        int atlasHeight_ = 0;
        float pixelHeight_ = 0.0f;
        std::array<stbtt_bakedchar, kNumChars> chars_{};
    };
} // namespace Forge
