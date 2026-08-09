#version 460 core
layout(location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    TexCoords = aPos;

    mat4 view = mat4(mat3(uView));
    vec4 position = uProjection * view * vec4(aPos, 1.0);

    gl_Position = position.xyww;
}
