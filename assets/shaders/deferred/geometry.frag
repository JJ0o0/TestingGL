#version 460 core

#include "../include/material.glsl"

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec4 gNormalRoughness;
layout(location = 2) out vec4 gAlbedoMetallic;
layout(location = 3) out vec4 gEmissiveAO;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 Tangent;

void main() {
    // BASE COLOR
    vec4 baseColor = texture(uBaseColorTexture, TexCoord) * uMaterial.BaseColor;

    if (uMaterial.AlphaMode == ALPHA_MODE_MASK && baseColor.a < uMaterial.AlphaCutoff) {
        discard;
    }

    // ARM
    vec3 arm = texture(uARMTexture, TexCoord).rgb;
    float ao = mix(1.0, arm.r, uMaterial.OcclusionStrength);
    float roughness = arm.g * uMaterial.Roughness;
    float metallic = arm.b * uMaterial.Metallic;

    roughness = clamp(roughness, 0.0, 1.0);
    metallic = clamp(metallic, 0.0, 1.0);

    // NORMAL
    vec3 N = normalize(Normal);
    vec3 T = normalize(Tangent.xyz - N * dot(N, Tangent.xyz));
    vec3 B = cross(N, T) * Tangent.w;
    mat3 TBN = mat3(T, B, N);

    vec3 tangentNormal = texture(uNormalTexture, TexCoord).rgb;
    tangentNormal = tangentNormal * 2.0 - 1.0;
    tangentNormal.xy *= uMaterial.NormalScale;

    tangentNormal = normalize(tangentNormal);

    N = normalize(TBN * tangentNormal);

    // EMISSIVE
    vec3 emissive = texture(uEmissiveTexture, TexCoord).rgb;
    emissive *= uMaterial.EmissiveColor.rgb * uMaterial.EmissiveStrength;

    // G-BUFFER OUTPUT
    gPosition = FragPos;
    gNormalRoughness = vec4(N, roughness);
    gAlbedoMetallic = vec4(baseColor.rgb, metallic);
    gEmissiveAO = vec4(emissive, ao);
}
