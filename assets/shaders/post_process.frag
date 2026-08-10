#version 460 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D uScene;

uniform float uExposure;
uniform int uToneMapping;

const int TONE_MAPPING_NONE = 0;
const int TONE_MAPPING_REINHARD = 1;
const int TONE_MAPPING_ACES = 2;

vec3 Reinhard(vec3 color) {
    return color / (color + vec3(1.0));
}

vec3 ACES(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

void main() {
    vec4 hdr = texture(uScene, TexCoord);
    vec3 color = hdr.rgb * uExposure;

    switch (uToneMapping) {
        case TONE_MAPPING_REINHARD:
            color = Reinhard(color);
            break;
        case TONE_MAPPING_ACES:
            color = ACES(color);
            break;
        case TONE_MAPPING_NONE:
        default:
            color = clamp(color, 0.0, 1.0);
            break;
    }

    FragColor = vec4(color, hdr.a);
}
