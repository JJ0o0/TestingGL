struct DirectionalLight {
    vec3 Tint;
    float Intensity;
    vec3 Direction;
};

struct PointLight {
    vec3 Tint;
    float Intensity;
    vec3 Position;
    float Radius;
};

struct SpotLight {
    vec3 Tint;
    float Intensity;
    vec3 Position;
    vec3 Direction;
    float Radius;
    float InnerCutoff;
    float OuterCutoff;
};

uniform DirectionalLight uDirectionalLight;

const int MAX_POINT_LIGHTS = 16;
uniform PointLight uPointLights[MAX_POINT_LIGHTS];
uniform int uPointLightCount;

const int MAX_SPOT_LIGHTS = 16;
uniform SpotLight uSpotLights[MAX_SPOT_LIGHTS];
uniform int uSpotLightCount;

vec3 CalculateDirectionalLight(
    DirectionalLight light,
    vec3 baseColor, float metallic, float roughness,
    vec3 N, vec3 V
) {
    vec3 L = normalize(-light.Direction);
    vec3 radiance = light.Tint * light.Intensity;

    return CalculatePBR(
        baseColor, metallic, roughness,
        N, V, L,
        radiance
    );
}

vec3 CalculatePointLight(
    PointLight light, vec3 fragPos,
    vec3 baseColor, float metallic, float roughness,
    vec3 N, vec3 V
) {
    vec3 toLight = light.Position - fragPos;
    float distance = length(toLight);

    if (distance <= 0.0001) return vec3(0.0);
    if (distance > light.Radius) return vec3(0.0);

    vec3 L = toLight / distance;
    float attenuation = 1.0 - (distance / light.Radius);
    attenuation *= attenuation;

    vec3 radiance = light.Tint * light.Intensity * attenuation;

    return CalculatePBR(
        baseColor, metallic, roughness,
        N, V, L,
        radiance
    );
}

vec3 CalculateSpotLight(
    SpotLight light, vec3 fragPos,
    vec3 baseColor, float metallic, float roughness,
    vec3 N, vec3 V
) {
    vec3 toLight = light.Position - fragPos;
    float distance = length(toLight);

    if (distance <= 0.0001) return vec3(0.0);
    if (distance > light.Radius) return vec3(0.0);

    vec3 L = toLight / distance;
    float attenuation = 1.0 - (distance / light.Radius);
    attenuation *= attenuation;

    vec3 lightToFrag = -L;

    float theta = dot(lightToFrag, normalize(light.Direction));
    if (theta <= light.OuterCutoff) return vec3(0.0);

    float spotAttenuation = (theta - light.OuterCutoff) / (light.InnerCutoff - light.OuterCutoff);
    spotAttenuation = clamp(spotAttenuation, 0.0, 1.0);

    vec3 radiance = light.Tint * light.Intensity * attenuation * spotAttenuation;

    return CalculatePBR(
        baseColor, metallic, roughness,
        N, V, L,
        radiance
    );
}
