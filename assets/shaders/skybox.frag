#version 460 core
out vec4 FragColor;
in vec3 TexCoords;

uniform samplerCube uEnvironmentMap;

void main() {
    vec3 color = texture(uEnvironmentMap, TexCoords).rgb;
    color = color / (color + vec3(1.0)); // REINHARD
    FragColor = vec4(color, 1.0);
}
