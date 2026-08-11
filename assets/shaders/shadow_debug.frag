#version 460 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D uDepthMap;

void main() {
    float depth = texture(uDepthMap, TexCoord).r;
    FragColor = vec4(vec3(depth), 1.0);
}
