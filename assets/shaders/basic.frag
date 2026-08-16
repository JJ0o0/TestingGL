#version 460 core

#include "include/material.glsl"
#include "include/pbr.glsl"
#include "include/lighting.glsl"

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 Tangent;

uniform float uEnvironmentIntensity;
uniform vec3 uCameraPosition;
uniform mat4 uLightSpaceMatrix;

uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;

uniform sampler2D uBRDFLUT;
uniform sampler2D uShadowMap;

float CalculateShadow(vec3 worldPos, vec3 normal, vec3 lightDir) {
    vec4 fragPosLightSpace = uLightSpaceMatrix * vec4(worldPos, 1.0);

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (
        projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0
    ) {
        return 0.0;
    }

    float currentDepth = projCoords.z;

    float NdotL = max(dot(normal, lightDir), 0.0);
    float minBias = 0.0005;
    float maxBias = 0.005;
    float bias = max(maxBias * (1.0 - NdotL), minBias);

    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    float shadow = 0.0;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closestDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

void main() {
    vec4 baseColorSample = texture(uBaseColorTexture, TexCoord);
    vec4 baseColor = baseColorSample * uMaterial.BaseColor;

    if (uMaterial.AlphaMode == ALPHA_MODE_MASK && baseColor.a < uMaterial.AlphaCutoff) {
        discard;
    }

    vec3 arm = texture(uARMTexture, TexCoord).rgb;

    float ao = mix(1.0, arm.r, uMaterial.OcclusionStrength);
    float roughness = arm.g * uMaterial.Roughness;
    float metallic = arm.b * uMaterial.Metallic;
    roughness = clamp(roughness, 0.0, 1.0);
    metallic = clamp(metallic, 0.0, 1.0);

    vec3 geometryNormal = normalize(Normal);
    vec3 N = geometryNormal; // NORMAL
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
    vec3 ibl = (diffuseIBL * ao + specularIBL) * uEnvironmentIntensity;

    vec3 directional = CalculateDirectionalLight(
        uDirectionalLight,
        baseColor.rgb, metallic, roughness,
        N, V
    );

    vec3 emissiveSample = texture(uEmissiveTexture, TexCoord).rgb;
    vec3 emissive = emissiveSample * uMaterial.EmissiveColor.rgb * uMaterial.EmissiveStrength;

    vec3 L = normalize(-uDirectionalLight.Direction);
    float shadow = CalculateShadow(FragPos, geometryNormal, L);

    vec3 pointLighting = vec3(0.0);
    for (int i = 0; i < uPointLightCount; ++i) {
        pointLighting += CalculatePointLight(
            uPointLights[i],
            FragPos,
            baseColor.rgb,
            metallic,
            roughness,
            N,
            V
        );
    }

    vec3 spotLighting = vec3(0.0);
    for (int i = 0; i < uSpotLightCount; ++i) {
        spotLighting += CalculateSpotLight(
            uSpotLights[i],
            FragPos,
            baseColor.rgb,
            metallic,
            roughness,
            N,
            V
        );
    }

    vec3 result = ibl + directional * (1.0 - shadow) + pointLighting + spotLighting + emissive;
    FragColor = vec4(result, baseColor.a);
}
