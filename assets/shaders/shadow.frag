#version 460 core

#include "include/material.glsl"

in vec2 TexCoord;

void main() {
    if (uMaterial.AlphaMode == ALPHA_MODE_MASK) {
        float alpha = texture(uBaseColorTexture, TexCoord).a * uMaterial.BaseColor.a;
        if (alpha < uMaterial.AlphaCutoff) discard;
    }
}
