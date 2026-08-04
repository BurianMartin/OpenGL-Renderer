#include "Forge/Rendering/Font.hpp"

// Implementation lives in exactly this one translation unit — same pattern as
// Texture.cpp's STB_IMAGE_IMPLEMENTATION. Do not add this define anywhere else.
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype/stb_truetype.h"

#include <fstream>
#include <vector>

namespace Forge
{
    namespace
    {
        // Fixed square atlas, large enough to fit the full printable-ASCII range at any
        // reasonable UI font size. stbtt_BakeFontBitmap reports (via a non-positive return)
        // if it doesn't, rather than silently truncating.
        constexpr int kAtlasSize = 512;
    }

    std::shared_ptr<Font> Font::Create(const std::string &ttfPath, float pixelHeight)
    {
        if (pixelHeight <= 0.0f)
        {
            debug_warn("Font::Create: invalid pixel height");
            return nullptr;
        }

        std::ifstream file(ttfPath, std::ios::binary | std::ios::ate);
        if (!file)
        {
            debug_warn("Font::Create: failed to open " << ttfPath);
            return nullptr;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<unsigned char> ttfBuffer(static_cast<size_t>(size));
        if (!file.read(reinterpret_cast<char *>(ttfBuffer.data()), size))
        {
            debug_warn("Font::Create: failed to read " << ttfPath);
            return nullptr;
        }

        std::vector<unsigned char> bitmap(static_cast<size_t>(kAtlasSize) * kAtlasSize, 0);

        auto font = std::shared_ptr<Font>(new Font());
        int result = stbtt_BakeFontBitmap(ttfBuffer.data(), 0, pixelHeight,
                                          bitmap.data(), kAtlasSize, kAtlasSize,
                                          kFirstChar, kNumChars, font->chars_.data());
        if (result <= 0)
        {
            debug_warn("Font::Create: " << ttfPath << " at " << pixelHeight
                                        << "px didn't fit the atlas (stbtt_BakeFontBitmap returned " << result << ")");
            return nullptr;
        }

        font->pixelHeight_ = pixelHeight;
        font->atlasWidth_ = kAtlasSize;
        font->atlasHeight_ = kAtlasSize;

        glGenTextures(1, &font->atlasTexture_);
        glBindTexture(GL_TEXTURE_2D, font->atlasTexture_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // atlas is single-channel — never assume 4-byte row alignment
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, kAtlasSize, kAtlasSize, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        return font;
    }

    Font::~Font()
    {
        if (atlasTexture_ != 0)
            glDeleteTextures(1, &atlasTexture_);
    }

    const stbtt_bakedchar &Font::GetGlyph(char c) const
    {
        int index = static_cast<int>(static_cast<unsigned char>(c)) - kFirstChar;
        if (index < 0 || index >= kNumChars)
            index = '?' - kFirstChar;
        return chars_[static_cast<size_t>(index)];
    }
} // namespace Forge
