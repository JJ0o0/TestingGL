#version 460 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D uTexture;

void main() {
    float value = texture(uTexture, TexCoord).a;
    FragColor = vec4(vec3(value), 1.0);
}
