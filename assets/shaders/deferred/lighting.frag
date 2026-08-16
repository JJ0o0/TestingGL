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

// SSAO
uniform sampler2D uSSAO;

// Probes
#define MAX_REFLECTION_PROBES 8

struct ReflectionProbe {
    vec3 Position;
    vec3 BoxMin;
    vec3 BoxMax;
    float Intensity;
    float BlendDistance;
};

uniform int uReflectionProbeCount;

uniform ReflectionProbe uReflectionProbes[MAX_REFLECTION_PROBES];
uniform samplerCube uProbeIrradianceMaps[MAX_REFLECTION_PROBES];
uniform samplerCube uProbePrefilterMaps[MAX_REFLECTION_PROBES];

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

vec3 BoxProjectDirection(
    vec3 pos, vec3 dir,
    vec3 probePos, vec3 boxMin, vec3 boxMax
) {
    vec3 t0 = (boxMin - pos) / dir;
    vec3 t1 = (boxMax - pos) / dir;

    vec3 tFar3 = max(t0, t1);

    float tFar = min(tFar3.x, min(tFar3.y, tFar3.z));
    vec3 hitPos = pos + dir * tFar;

    return hitPos - probePos;
}

float CalculateProbeWeight(
    vec3 pos,
    vec3 boxMin, vec3 boxMax,
    float blendDistance
) {
    vec3 distToEdge3 = min(pos - boxMin, boxMax - pos);
    float distToEdge = min(distToEdge3.x, min(distToEdge3.y, distToEdge3.z));
    if (distToEdge <= 0.0) return 0.0;
    if (blendDistance <= 0.0) return 1.0;

    return clamp(distToEdge / blendDistance, 0.0, 1.0);
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

    const float materialAO = emissiveAO.a;
    const float ssao = texture(uSSAO, TexCoord).r;
    const float ao = materialAO * ssao;

    // VIEW
    const vec3 V = normalize(uCameraPosition - fragPos);

    // IBL
    const vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    const float NdotV = max(dot(N, V), 0.0);
    const vec3 R = reflect(-V, N);
    const vec2 brdf = texture(uBRDFLUT, vec2(NdotV, roughness)).rg;
    const vec3 F = FresnelSchlickRoughness(NdotV, F0, roughness);
    const float MAX_REFLECTION_LOD = 4.0;

    const vec3 environmentIrradiance = texture(uIrradianceMap, N).rgb;
    const vec3 environmentDiffuse = CalculateDiffuseIBL(
        environmentIrradiance,
        baseColor, metallic, roughness,
        N, V
    );

    const vec3 environmentPrefiltered = textureLod(uPrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    const vec3 environmentSpecular = environmentPrefiltered * (F * brdf.x + brdf.y);

    vec3 environmentIBL = (environmentDiffuse * ao + environmentSpecular) * uEnvironmentIntensity;

    vec3 accumulatedProbeIBL = vec3(0.0);
    float totalWeight = 0.0;
    for (int i = 0; i < uReflectionProbeCount; ++i) {
        const ReflectionProbe probe = uReflectionProbes[i];

        const float weight = CalculateProbeWeight(fragPos, probe.BoxMin, probe.BoxMax, probe.BlendDistance);
        if (weight <= 0.0) continue;

        vec3 irradiance = texture(uProbeIrradianceMaps[i], N).rgb;
        vec3 diffuse = CalculateDiffuseIBL(
            irradiance,
            baseColor, metallic, roughness,
            N, V
        );

        vec3 probeR = BoxProjectDirection(fragPos, R, probe.Position, probe.BoxMin, probe.BoxMax);
        vec3 prefiltered = textureLod(uProbePrefilterMaps[i], probeR, roughness * MAX_REFLECTION_LOD).rgb;
        vec3 specular = prefiltered * (F * brdf.x + brdf.y);

        vec3 probeIBL = (diffuse * ao + specular) * probe.Intensity;
        accumulatedProbeIBL += probeIBL * weight;
        totalWeight += weight;
    }

    vec3 ibl = environmentIBL;
    if (totalWeight > 0.0) {
        const vec3 probeIBL = accumulatedProbeIBL / totalWeight;
        const float probeInfluence = clamp(totalWeight, 0.0, 1.0);
        ibl = mix(environmentIBL, probeIBL, probeInfluence);
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

    // POINT LIGHT
    vec3 pointLighting = vec3(0.0);
    for (int i = 0; i < uPointLightCount; ++i) {
        pointLighting += CalculatePointLight(
            uPointLights[i],
            fragPos,
            baseColor,
            metallic,
            roughness,
            N,
            V
        );
    }

    // SPOT LIGHT
    vec3 spotLighting = vec3(0.0);
    for (int i = 0; i < uSpotLightCount; ++i) {
        spotLighting += CalculateSpotLight(
            uSpotLights[i],
            fragPos,
            baseColor,
            metallic,
            roughness,
            N,
            V
        );
    }

    // SHADOW
    const vec3 L = normalize(-uDirectionalLight.Direction);
    const float shadow = CalculateShadow(
        fragPos,
        N,
        L
    );

    const vec3 result = ibl + directional * (1.0 - shadow) + pointLighting + spotLighting + emissive;
    FragColor = vec4(result, 1.0);
}
