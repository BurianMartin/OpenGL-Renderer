#version 450 core

// Pairs with the shared vertex.glsl. Deliberately minimal — no lighting, no texture, no
// vNormal/vFragPos/vTexCoords inputs at all — just outputs the material's flat color.
// Material::Bind() still uploads uMaterial/uHasTexture/uCameraPos/etc every frame, but
// glUniform* on a name this shader never declares (GetUniformLocation returns -1) is a
// silent, well-defined no-op, so none of that unused state needs declaring here.

out vec4 FragColor;

uniform vec4 uBaseColor;

void main()
{
    FragColor = uBaseColor;
}
