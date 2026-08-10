#version 460 core

#include "include/material.glsl"

out vec4 FragColor;
in vec2 TexCoord;

void main() {
    vec4 baseColor = texture(uBaseColorTexture, TexCoord) * uMaterial.BaseColor;
    FragColor = baseColor;
}
