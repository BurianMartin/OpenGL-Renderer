#version 450 core
in vec2 vNDC;
out vec4 FragColor;

uniform mat4 uInvViewProj; // inverse(projection * view) — reconstructs the view ray per-pixel, no mesh needed

void main()
{
    vec4 nearP4 = uInvViewProj * vec4(vNDC, -1.0, 1.0);
    vec4 farP4 = uInvViewProj * vec4(vNDC, 1.0, 1.0);
    vec3 nearP = nearP4.xyz / nearP4.w;
    vec3 farP = farP4.xyz / farP4.w;
    vec3 dir = normalize(farP - nearP);

    vec3 zenith = vec3(0.20, 0.40, 0.80);
    vec3 horizon = vec3(0.80, 0.88, 0.95);
    vec3 haze = vec3(0.9, 0.92, 0.9);

    float t = clamp(dir.y, 0.0, 1.0);
    vec3 color = mix(horizon, zenith, t);

    // Horizon haze band — thickest right at the horizon, thinning out toward zenith/ground.
    float hazeAmount = exp(-abs(dir.y) * 8.0);
    color = mix(color, haze, hazeAmount * 0.6);

    FragColor = vec4(color, 1.0);
}
