#version 460 core
out vec4 FragColor;
in vec3 LocalPos;

uniform samplerCube uEnvironmentMap;
uniform float uRoughness;

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) |
           ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) |
           ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) |
           ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) |
           ((bits & 0xFF00FF00u) >> 8u);

    return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint sampleCount) {
    return vec2(
        float(i) / float(sampleCount),
        RadicalInverse_VdC(i)
    );
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float alpha = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;

    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha * alpha - 1.0) * Xi.y));
    float sinTheta = sqrt(max(1.0 - cosTheta * cosTheta, 0.0));

    vec3 H = vec3(
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta
    );

    vec3 helper =
        abs(N.z) < 0.999
            ? vec3(0.0, 0.0, 1.0)
            : vec3(1.0, 0.0, 0.0);

    vec3 tangent = normalize(cross(helper, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleDirection =
        tangent * H.x +
        bitangent * H.y +
        N * H.z;

    return normalize(sampleDirection);
}

float DistributionGGX(float NdotH, float roughness) {
    float alpha = max(roughness * roughness, 0.001);
    float alpha2 = alpha * alpha;

    float dem = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
    dem = PI * dem * dem;

    return alpha2 / dem;
}

void main() {
    vec3 N = normalize(LocalPos);

    vec3 R = N;
    vec3 V = R;

    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;

    const uint SAMPLE_COUNT = 512u;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);

        vec3 H = ImportanceSampleGGX(Xi, N, uRoughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);

        if (NdotL > 0.0) {
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);

            float D = DistributionGGX(NdotH, uRoughness);

            float pdf = D * NdotH / max(4.0 * HdotV, 0.0001);
            pdf = max(pdf, 0.0001);

            float resolution = float(textureSize(uEnvironmentMap, 0).x);

            float texelSolidAngle = 4.0 * PI / (6.0 * resolution * resolution);
            float sampleSolidAngle = 1.0 / (float(SAMPLE_COUNT) * pdf);

            float mipLevel = uRoughness == 0.0
                             ? 0.0
                             : 0.5 * log2(sampleSolidAngle / texelSolidAngle);

            prefilteredColor += textureLod(uEnvironmentMap, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    prefilteredColor /= max(totalWeight, 0.0001);
    FragColor = vec4(prefilteredColor, 1.0);
}
