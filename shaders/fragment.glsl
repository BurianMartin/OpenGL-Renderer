#version 450 core
in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoords;

out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material uMaterial;

uniform vec4 uBaseColor;
uniform bool uHasTexture;
uniform sampler2D uTexture;
uniform vec3 uCameraPos;
uniform float uTime;

void main()
{
    vec4 baseColor = uHasTexture ? texture(uTexture, vTexCoords) : uBaseColor;

    vec3 norm     = normalize(vNormal);
    vec3 lightDir = normalize(vec3(2.0, 3.0, 1.5));
    vec3 viewDir  = normalize(uCameraPos - vFragPos);
    vec3 halfDir  = normalize(lightDir + viewDir);

    // ambient — scaled down as placeholder for light ambient weight (replaced by UBO)
    vec3 ambient = uMaterial.ambient * baseColor.rgb;

    // diffuse — how directly the surface faces the light
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = uMaterial.diffuse * diff * baseColor.rgb;

    // specular — the highlight
    float spec = pow(max(dot(norm, halfDir), 0.0), uMaterial.shininess);
    vec3 specular = uMaterial.specular * spec;

    FragColor = vec4(clamp(ambient + diffuse + specular, 0.0, 1.0), baseColor.a);
}