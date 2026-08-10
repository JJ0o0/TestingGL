#version 460 core

#include "include/material.glsl"
#include "include/pbr.glsl"
#include "include/lighting.glsl"

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 Tangent;

uniform vec3 uCameraPosition;
uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uBRDFLUT;

void main() {
    vec4 baseColorSample = texture(uBaseColorTexture, TexCoord);
    vec4 baseColor = baseColorSample * uMaterial.BaseColor;

    vec3 arm = texture(uARMTexture, TexCoord).rgb;

    float ao = mix(1.0, arm.r, uMaterial.OcclusionStrength);
    float roughness = arm.g * uMaterial.Roughness;
    float metallic = arm.b * uMaterial.Metallic;
    roughness = clamp(roughness, 0.0, 1.0);
    metallic = clamp(metallic, 0.0, 1.0);

    vec3 N = normalize(Normal); // NORMAL
    vec3 T = normalize(Tangent.xyz - N * dot(N, Tangent.xyz)); // TANGENT
    vec3 B = cross(N, T) * Tangent.w; // BITANGENT
    mat3 TBN = mat3(T, B, N);

    vec3 tangentNormal = texture(uNormalTexture, TexCoord).rgb;
    tangentNormal = tangentNormal * 2.0 - 1.0;
    tangentNormal.xy *= uMaterial.NormalScale;
    tangentNormal = normalize(tangentNormal);

    N = normalize(TBN * tangentNormal);

    vec3 V = normalize(uCameraPosition - FragPos); // VIEW DIRECTION

    vec3 irradiance = texture(uIrradianceMap, N).rgb;

    vec3 diffuseIBL = CalculateDiffuseIBL(
        irradiance,
        baseColor.rgb, metallic, roughness,
        N, V
    );

    vec3 F0 = mix(vec3(0.04), baseColor.rgb, metallic);
    float NdotV = max(dot(N, V), 0.0);
    vec3 R = reflect(-V, N);

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(uPrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(uBRDFLUT, vec2(NdotV, roughness)).rg;

    vec3 F = FresnelSchlickRoughness(NdotV, F0, roughness);

    vec3 specularIBL = prefilteredColor * (F * brdf.x + brdf.y);
    vec3 ambient = diffuseIBL * ao + specularIBL;

    vec3 directional = CalculateDirectionalLight(
        uDirectionalLight,
        baseColor.rgb, metallic, roughness,
        N, V
    );

    vec3 emissiveSample = texture(uEmissiveTexture, TexCoord).rgb;
    vec3 emissive = emissiveSample * uMaterial.EmissiveColor.rgb * uMaterial.EmissiveStrength;

    vec3 result = ambient + directional + emissive;
    FragColor = vec4(result, baseColor.a);
}
