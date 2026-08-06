#version 450 core

in vec2 vLocal; // 0..1 across the rect, regardless of its pixel size -- see Panel::Rebuild()
out vec4 FragColor;

uniform vec4 uFillColorTop;    // fill color at vLocal.y == 0 (the rect's top edge)
uniform vec4 uFillColorBottom; // fill color at vLocal.y == 1 (the rect's bottom edge)
uniform bool uHasTexture;      // same uHasTexture/uTexture naming convention as fragment.glsl/Material
uniform sampler2D uTexture;
uniform vec4 uBorderColor;
uniform float uBorderThicknessPx; // real pixel units now, not a UV fraction -- 0 disables the border entirely
uniform vec2 uSizePx;             // Panel size in pixels, needed to turn vLocal back into a real distance
uniform float uCornerRadiusPx;    // 0 is a plain sharp-cornered rect

// Inigo Quilez's rounded-box SDF: signed distance from p (relative to the box's center) to
// a box of half-size b with corner radius r. Negative inside the shape, 0 on its outline,
// positive outside.
float RoundedBoxSDF(vec2 p, vec2 b, float r)
{
    vec2 q = abs(p) - b + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

void main()
{
    vec2 pixelPos = vLocal * uSizePx;
    vec2 halfSize = uSizePx * 0.5;
    float dist = RoundedBoxSDF(pixelPos - halfSize, halfSize, uCornerRadiusPx);

    // A 1px antialiased falloff at the silhouette instead of a hard discard -- keeps rounded
    // corners (and, at uCornerRadiusPx == 0, the plain rectangular case) from looking jagged.
    float coverage = 1.0 - smoothstep(-1.0, 1.0, dist);
    if (coverage <= 0.0)
        discard;

    // A textured Panel (card art) is tinted by uFillColorTop alone (e.g. a domain-color wash)
    // rather than the vertical gradient, which would otherwise muddy the art; an untextured
    // Panel keeps the existing top/bottom gradient fill exactly as before.
    vec4 fill = uHasTexture ? texture(uTexture, vLocal) * uFillColorTop : mix(uFillColorTop, uFillColorBottom, vLocal.y);
    bool inBorder = uBorderThicknessPx > 0.0 && dist > -uBorderThicknessPx;
    vec4 color = inBorder ? uBorderColor : fill;
    color.a *= coverage;

    FragColor = color;
}
