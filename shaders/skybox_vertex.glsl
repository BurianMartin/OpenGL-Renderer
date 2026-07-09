#version 450 core
layout(location = 0) in vec3 aPos;

uniform mat4 uView;       // rotation-only — caller strips translation so the cube always surrounds the camera
uniform mat4 uProjection;

out vec3 vDirection;

void main()
{
    vDirection = aPos; // the unit cube's own position doubles as the view direction
    vec4 pos = uProjection * uView * vec4(aPos, 1.0);
    // Force z/w == 1.0 (the far plane) — guarantees the skybox draws behind everything else.
    gl_Position = vec4(pos.xy, pos.w, pos.w);
}
