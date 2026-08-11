#version 460 core

#include "../include/pbr.glsl"
#include "../include/lighting.glsl"

out vec4 FragColor;
in vec2 TexCoord;

// GBuffer
uniform sampler2D uGPosition;
uniform sampler2D uGNormalRoughness;
uniform sampler2D uGAlbedoMetallic;
uniform sampler2D uGEmissiveAO;

// IBL
uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uBRDFLUT;

// Shadows
uniform sampler2D uShadowMap;
uniform mat4 uLightSpaceMatrix;

// Scene
uniform vec3 uCameraPosition;
uniform float uEnvironmentIntensity;

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

    const float currentDepth = projCoords.z;
    const float NdotL = max(dot(normal, lightDir), 0.0);

    const float minBias = 0.0005;
    const float maxBias = 0.005;

    const float bias = max(maxBias * (1.0 - NdotL), minBias);

    const vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);

    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            const float closestDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}


void main() {
    // READ GBUFFER
    const vec3 fragPos = texture(uGPosition, TexCoord).rgb;
    const vec4 normalRoughness = texture(uGNormalRoughness, TexCoord);

    if (dot(normalRoughness.xyz, normalRoughness.xyz) < 0.0001) {
        discard;
    }

    const vec4 albedoMetallic = texture(uGAlbedoMetallic, TexCoord);
    const vec4 emissiveAO = texture(uGEmissiveAO, TexCoord);

    const vec3 N = normalize(normalRoughness.xyz);
    const float roughness = normalRoughness.a;
    const vec3 baseColor = albedoMetallic.rgb;
    const float metallic = albedoMetallic.a;
    const vec3 emissive = emissiveAO.rgb;
    const float ao = emissiveAO.a;

    // VIEW
    const vec3 V = normalize(uCameraPosition - fragPos);

    // IBL
    vec3 ibl = vec3(0.0);

    if (uEnvironmentIntensity > 0.0) {
        const vec3 irradiance = texture(uIrradianceMap, N).rgb;
        const vec3 diffuseIBL = CalculateDiffuseIBL(
            irradiance,
            baseColor,
            metallic,
            roughness,
            N,
            V
        );

        const vec3 F0 = mix(vec3(0.04), baseColor, metallic);
        const float NdotV = max(dot(N, V), 0.0);
        const vec3 R = reflect(-V, N);

        const float MAX_REFLECTION_LOD = 4.0;

        const vec3 prefilteredColor = textureLod(uPrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
        const vec2 brdf = texture(uBRDFLUT, vec2(NdotV, roughness)).rg;

        const vec3 F = FresnelSchlickRoughness(
            NdotV,
            F0,
            roughness
        );

        const vec3 specularIBL = prefilteredColor * (F * brdf.x + brdf.y);

        ibl = (diffuseIBL * ao + specularIBL) * uEnvironmentIntensity;
    }

    // DIRECT LIGHT
    const vec3 directional = CalculateDirectionalLight(
        uDirectionalLight,
        baseColor,
        metallic,
        roughness,
        N,
        V
    );

    // SHADOW

    const vec3 L = normalize(-uDirectionalLight.Direction);
    const float shadow = CalculateShadow(
        fragPos,
        N,
        L
    );

    const vec3 result = ibl + directional * (1.0 - shadow) + emissive;
    FragColor = vec4(result, 1.0);
}
