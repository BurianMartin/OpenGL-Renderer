#version 450 core
out vec2 vNDC;

void main()
{
    // Deliberately no bound vertex attributes/buffers — a full-screen triangle built purely
    // from gl_VertexID, oversized so the rasterizer clips it to exactly the visible area.
    vec2 positions[3] = vec2[](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));
    vNDC = positions[gl_VertexID];
    gl_Position = vec4(vNDC, 1.0, 1.0); // far plane, same trick as the skybox
}
