#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 uModel;
uniform mat4 uLightSpaceMatrix;

void main() {
    TexCoord = aTexCoord;
    gl_Position = uLightSpaceMatrix * uModel * vec4(aPos, 1.0);
}
