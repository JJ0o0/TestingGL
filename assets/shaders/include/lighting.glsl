struct DirectionalLight {
    vec3 Tint;
    float Intensity;
    vec3 Direction;
};

uniform DirectionalLight uDirectionalLight;

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
