#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoords;

// Deliberately no aNormal (location 1) — same reasoning as text_vertex.glsl: Mesh::setup()
// always binds it, but a flat screen-space rect has no use for one.

uniform mat4 uProjection; // fixed screen-space-pixels orthographic projection, rebuilt per Panel::Draw() call from the current window size

out vec2 vLocal; // 0..1 across the rect regardless of its pixel size — see Panel::Rebuild()

void main()
{
    gl_Position = uProjection * vec4(aPos.xy, 0.0, 1.0);
    vLocal = aTexCoords;
}
