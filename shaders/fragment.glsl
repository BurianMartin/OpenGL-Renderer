#version 450 core
#define MAX_LIGHTS 32

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

struct Light {
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 args1;
    vec4 args2;
};

layout(std140, binding = 0) uniform LightsBlock {
    vec4 uLightCountHeader;
    Light lights[MAX_LIGHTS];
};

vec3 DirectionLightValue(Light l, vec4 color, vec3 norm, vec3 viewDir);
vec3 PointLightValue(Light l, vec4 color, vec3 norm, vec3 viewDir);
vec3 SpotLightValue(Light l, vec4 color, vec3 norm, vec3 viewDir);

void main()
{
    vec4 baseColor = uHasTexture ? texture(uTexture, vTexCoords) : uBaseColor;
    vec3 viewDir  = normalize(uCameraPos - vFragPos);
    vec3 norm     = normalize(vNormal);
    vec3 totalLight = vec3(0);

    for(int i = 0; i < uLightCountHeader.x; ++i)
    {
        switch (int(lights[i].position.w)) {
            case 0: // Light::Directional
            {
                totalLight += DirectionLightValue(lights[i], baseColor, norm, viewDir);
                break;
            }

            case 1: // Light::Point
            {
                totalLight += PointLightValue(lights[i], baseColor, norm, viewDir);
                break;
            }

            case 2: // Light::Spot
            {
                totalLight += SpotLightValue(lights[i], baseColor, norm, viewDir);
                break;
            }
            default:
                break;
        }
    }
    FragColor = vec4(totalLight, uBaseColor.a);
}



vec3 DirectionLightValue(Light l, vec4 color, vec3 norm, vec3 viewDir){

    // l.direction stores the direction the light travels (e.g. sunlight
    // pointing down); the lighting math needs the fragment-to-light
    // direction instead, so negate it.
    vec3 lightDir = normalize(-l.direction.xyz);
    vec3 halfDir  = normalize(lightDir + viewDir);
    float intensity = l.direction.w;

    // ambient — scaled down as placeholder for light ambient weight (replaced by UBO)
    vec3 ambient = uMaterial.ambient * color.rgb * l.color.rgb * intensity;

    // diffuse — how directly the surface faces the light
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = uMaterial.diffuse * diff * color.rgb * l.color.rgb * intensity;

    // specular — the highlight
    float spec = pow(max(dot(norm, halfDir), 0.0), uMaterial.shininess);
    vec3 specular = uMaterial.specular * spec * l.color.rgb * intensity;

    vec3 total = ambient + diffuse + specular;

    return total;

}

vec3 PointLightValue(Light l, vec4 color, vec3 norm, vec3 viewDir){

    // Unlike a directional light, a point light has an actual position, so
    // the direction toward it is different for every fragment.
    vec3 lightDir = normalize(l.position.xyz - vFragPos);
    vec3 halfDir  = normalize(lightDir + viewDir);
    float intensity = l.direction.w;

    // Distance-based falloff: the further away, the dimmer.
    // args1 = (constant, linear, quadratic, innerCutoff) — see GPULight.
    float dist = length(l.position.xyz - vFragPos);
    float attenuation = 1.0 / (l.args1.x + l.args1.y * dist + l.args1.z * dist * dist);

    vec3 ambient = uMaterial.ambient * color.rgb * l.color.rgb * intensity;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = uMaterial.diffuse * diff * color.rgb * l.color.rgb * intensity;

    float spec = pow(max(dot(norm, halfDir), 0.0), uMaterial.shininess);
    vec3 specular = uMaterial.specular * spec * l.color.rgb * intensity;

    vec3 total = ambient + diffuse + specular;

    return total * attenuation;

}

vec3 SpotLightValue(Light l, vec4 color, vec3 norm, vec3 viewDir){

    // A spot light is a point light with an extra cone-shaped mask on top.
    vec3 lightDir = normalize(l.position.xyz - vFragPos);
    vec3 halfDir  = normalize(lightDir + viewDir);
    float intensity = l.direction.w;

    float dist = length(l.position.xyz - vFragPos);
    float attenuation = 1.0 / (l.args1.x + l.args1.y * dist + l.args1.z * dist * dist);

    // Cone falloff: l.direction.xyz is the direction the spotlight is aimed
    // (light -> forward). theta is high (close to 1) when the fragment sits
    // near the spotlight's central axis, and drops off toward its edges.
    // args1.w/args2.x are innerCutoff/outerCutoff, both stored as cosines —
    // see SpotLight::GetInnerCutoff()/GetOuterCutoff() on the C++ side.
    float theta = dot(lightDir, normalize(-l.direction.xyz));
    float epsilon = l.args1.w - l.args2.x;
    float coneFactor = clamp((theta - l.args2.x) / epsilon, 0.0, 1.0);

    vec3 ambient = uMaterial.ambient * color.rgb * l.color.rgb * intensity;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = uMaterial.diffuse * diff * color.rgb * l.color.rgb * intensity * coneFactor;

    float spec = pow(max(dot(norm, halfDir), 0.0), uMaterial.shininess);
    vec3 specular = uMaterial.specular * spec * l.color.rgb * intensity * coneFactor;

    // Ambient is deliberately NOT scaled by coneFactor, only by attenuation —
    // a fragment just outside the cone gets a faint ambient glow instead of
    // a harsh black edge exactly at the cone boundary.
    vec3 total = ambient + diffuse + specular;

    return total * attenuation;

}