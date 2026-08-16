#version 460 core

layout(location = 0) out float FragColor;
in vec2 TexCoord;

uniform sampler2D uGPosition;
uniform sampler2D uGNormalRoughness;
uniform sampler2D uNoiseTexture;

uniform mat4 uView;
uniform mat4 uProjection;

const int KERNEL_SIZE = 32;
uniform vec3 uSamples[KERNEL_SIZE];

uniform vec2 uNoiseScale;

uniform float uRadius;
uniform float uBias;

void main() {
    vec3 worldPos = texture(uGPosition, TexCoord).xyz;
    vec3 worldNormal = texture(uGNormalRoughness, TexCoord).xyz;
    if (length(worldNormal) < 0.0001) {
        FragColor = 1.0;
        return;
    }

    vec3 fragPos = vec3(uView * vec4(worldPos, 1.0));
    vec3 N = normalize(mat3(uView) * worldNormal);

    vec3 randomVec = texture(uNoiseTexture, TexCoord * uNoiseScale).xyz;

    vec3 T = normalize(randomVec - N * dot(randomVec, N));
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    float occlusion = 0.0;
    for (int i = 0; i < KERNEL_SIZE; ++i) {
        vec3 sampleDirection = TBN * uSamples[i];

        vec3 samplePos = fragPos + sampleDirection * uRadius;

        vec4 offset = uProjection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;

        vec2 sampleUV = offset.xy * 0.5 + 0.5;
        if (sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0) {
            continue;
        }

        vec3 sampledWorldPos = texture(uGPosition, sampleUV).xyz;
        vec3 sampledViewPos = vec3(uView * vec4(sampledWorldPos, 1.0));

        float rangeCheck = smoothstep(0.0, 1.0, uRadius / abs(fragPos.z - sampledViewPos.z));
        if (sampledViewPos.z >= samplePos.z + uBias) occlusion += rangeCheck;
    }

    float ao = 1.0 - occlusion / float(KERNEL_SIZE);
    FragColor = ao;
}
