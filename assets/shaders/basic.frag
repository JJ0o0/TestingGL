#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

struct AmbientLight {
    vec3 Tint;
    float Intensity;
};

struct DirectionalLight {
    vec3 Tint;
    float Intensity;
    vec3 Direction;
};

struct Material {
    vec4 Tint;
    sampler2D Diffuse;
    sampler2D Specular;
    float Shininess;
};

uniform Material uMaterial;
uniform AmbientLight uAmbient;
uniform DirectionalLight uDirectionalLight;

uniform vec3 uCameraPosition;

vec3 calculateDirectionalLightDiffuse(DirectionalLight light, vec3 N) {
    vec3 L = normalize(-light.Direction);
    float NdotL = max(dot(N, L), 0.0);

    vec3 diffuse = light.Tint * light.Intensity * NdotL;
    return diffuse;
}

vec3 calculateDirectionalLightSpecular(DirectionalLight light, vec3 N, vec3 V, vec2 uv) {
    vec3 L = normalize(-light.Direction);
    float NdotL = dot(N, L);

    if (NdotL <= 0.0) return vec3(0.0);

    vec3 R = reflect(-L, N);

    float spec = pow(max(dot(V, R), 0.0), uMaterial.Shininess);
    vec3 specularMap = texture(uMaterial.Specular, uv).rgb;

    return light.Tint * light.Intensity * spec * specularMap;
}

void main() {
    vec4 baseColor = texture(uMaterial.Diffuse, TexCoord);
    baseColor *= uMaterial.Tint;

    vec3 n = normalize(Normal);
    vec3 v = normalize(uCameraPosition - FragPos);

    vec3 ambient = uAmbient.Intensity * uAmbient.Tint;
    vec3 diffuse = calculateDirectionalLightDiffuse(uDirectionalLight, n);
    vec3 specular = calculateDirectionalLightSpecular(uDirectionalLight, n, v, TexCoord);

    vec3 result = baseColor.rgb * (ambient + diffuse) + specular;
    FragColor = vec4(result, baseColor.a);
}
