#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoords;

// Deliberately no aNormal (location 1) — Mesh::setup() always binds it, but text has no
// use for normals and the shader is free to simply not declare that input.

uniform mat4 uProjection; // fixed screen-space-pixels orthographic projection, rebuilt per Text::Draw() call from the current window size — not the scene camera's uProjection

out vec2 vTexCoords;

void main()
{
    gl_Position = uProjection * vec4(aPos.xy, 0.0, 1.0);
    vTexCoords = aTexCoords;
}
