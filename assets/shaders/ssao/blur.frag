#version 460 core

layout(location = 0) out float FragColor;
in vec2 TexCoord;

uniform sampler2D uSSAO;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(uSSAO, 0));

    float result = 0.0;
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            vec2 uvSample = TexCoord + vec2(x, y) * texelSize;
            result += texture(uSSAO, uvSample).r;
        }
    }

    result /= 25.0;
    FragColor = result;
}
