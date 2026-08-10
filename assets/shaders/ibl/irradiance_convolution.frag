#version 460 core
out vec4 FragColor;
in vec3 LocalPos;

uniform samplerCube uEnvironmentMap;

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

vec2 Hammersley(uint i, uint count) {
    return vec2(
        float(i) / float(count),
        RadicalInverse_VdC(i)
    );
}

vec3 CosineSampleHemisphere(vec2 Xi) {
    float phi = 2.0 * PI * Xi.x;

    float sinTheta = sqrt(Xi.y);
    float cosTheta = sqrt(1.0 - Xi.y);

    return vec3(
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta
    );
}

void main() {
    vec3 N = normalize(LocalPos);

    vec3 helper = abs(N.y) < 0.999
                  ? vec3(0.0, 1.0, 0.0)
                  : vec3(1.0, 0.0, 0.0);

    vec3 right = normalize(cross(helper, N));
    vec3 up = normalize(cross(N, right));

    vec3 irradiance = vec3(0.0);

    const uint SAMPLE_COUNT = 512u;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);

        vec3 tangentSample = CosineSampleHemisphere(Xi);

        vec3 sampleDirection =
            tangentSample.x * right +
            tangentSample.y * up +
            tangentSample.z * N;

        irradiance += texture(uEnvironmentMap, sampleDirection).rgb;
    }

    irradiance *= PI / float(SAMPLE_COUNT);
    FragColor = vec4(irradiance, 1.0);
}
