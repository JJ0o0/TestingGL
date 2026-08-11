const int ALPHA_MODE_OPAQUE = 0;
const int ALPHA_MODE_MASK = 1;
const int ALPHA_MODE_BLEND = 2;

struct Material {
    vec4 BaseColor;
    vec4 EmissiveColor;

    float Metallic;
    float Roughness;
    float NormalScale;
    float EmissiveStrength;
    float OcclusionStrength;

    int AlphaMode;
    float AlphaCutoff;
};

uniform Material uMaterial;

uniform sampler2D uBaseColorTexture;
uniform sampler2D uARMTexture;
uniform sampler2D uNormalTexture;
uniform sampler2D uEmissiveTexture;
