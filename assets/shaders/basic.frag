#version 460 core
out vec4 FragColor;
in vec2 TexCoord;

struct Material {
    vec4 Tint;
    sampler2D Diffuse;
};

uniform Material uMaterial;

void main() {
    vec4 baseColor = texture(uMaterial.Diffuse, TexCoord);
    baseColor *= uMaterial.Tint;

    vec4 result = baseColor;
    FragColor = result;
}
