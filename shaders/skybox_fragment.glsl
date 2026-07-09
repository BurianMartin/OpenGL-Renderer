#version 450 core
in vec3 vDirection;
out vec4 FragColor;

void main()
{
    vec3 dir = normalize(vDirection);

    vec3 zenith = vec3(0.25, 0.45, 0.85);
    vec3 horizon = vec3(0.75, 0.85, 0.95);
    vec3 ground = vec3(0.35, 0.32, 0.30);

    float t = clamp(dir.y, -1.0, 1.0);
    vec3 sky = mix(horizon, zenith, clamp(t, 0.0, 1.0));
    vec3 color = mix(ground, sky, smoothstep(-0.1, 0.05, t));

    // Sun disc, aimed opposite the scene's directional light direction (0.6, -1.0, 0.3) —
    // the light travels that way, so the sun itself sits at the reverse.
    vec3 sunDir = normalize(vec3(-0.6, 1.0, -0.3));
    float sun = pow(max(dot(dir, sunDir), 0.0), 400.0);
    color += vec3(1.0, 0.95, 0.8) * sun * 1.5;

    FragColor = vec4(color, 1.0);
}
