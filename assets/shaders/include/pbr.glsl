const float PI = 3.14159265359;

vec3 FresnelSchlick(float VdotH, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);
}

float DistributionGGX(float NdotH, float roughness) {
    float alpha = max(roughness * roughness, 0.001);
    float alpha2 = alpha * alpha;

    float dem = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
    dem = PI * dem * dem;

    return alpha2 / dem;
}

float GeometryShlickGGX(float NdotX, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotX / (NdotX * (1.0 - k) + k);
}

float GeometrySmith(float NdotV, float NdotL, float roughness) {
    float Gv = GeometryShlickGGX(NdotV, roughness);
    float Gl = GeometryShlickGGX(NdotL, roughness);
    return Gv * Gl;
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 CalculateDiffuseIBL(
    vec3 irradiance,
    vec3 baseColor, float metallic,
    vec3 N, vec3 V
) {
    float NdotV = max(dot(N, V), 0.0);

    vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    vec3 F = FresnelSchlick(NdotV, F0);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    return kD * irradiance * baseColor;
}

vec3 CalculatePBR(
    vec3 baseColor, float metallic, float roughness,
    vec3 N, vec3 V, vec3 L,
    vec3 radiance
) {
    vec3 H = normalize(V + L); // HALFWAY

    // DOT PRODUCTS
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    // FRESNEL SCHLICK
    vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    vec3 F = FresnelSchlick(VdotH, F0);

    // NORMAL DISTRIBUTION FUNCTION (GGX)
    float D = DistributionGGX(NdotH, roughness);

    // GEOMETRY (SCHLICK-GGX)
    float G = GeometrySmith(NdotV, NdotL, roughness);

    // COOK-TORRANCE
    vec3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, 0.0001);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuse = kD * baseColor / PI;

    return (diffuse + specular) * radiance * NdotL;
}
