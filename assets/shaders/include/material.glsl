struct Material {
    vec4 BaseColor;
    vec4 EmissiveColor;

    float Metallic;
    float Roughness;
    float NormalScale;
    float EmissiveStrength;
    float OcclusionStrength;
};

uniform Material uMaterial;

uniform sampler2D uBaseColorTexture;
uniform sampler2D uARMTexture;
uniform sampler2D uNormalTexture;
uniform sampler2D uEmissiveTexture;
