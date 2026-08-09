#version 460 core
out vec4 FragColor;

in vec3 LocalPos;

uniform sampler2D uEquirectangularMap;

const vec2 INV_ATAN = vec2(0.15915494309, 0.31830988618);
vec2 SampleSphericalMap(vec3 direction) {
    vec2 uv = vec2(
        atan(direction.z, direction.x),
        asin(direction.y)
    );

    uv *= INV_ATAN;
    uv += 0.5;
    return uv;
}

void main() {
    vec3 direction = normalize(LocalPos);
    vec2 uv = SampleSphericalMap(direction);

    vec3 color = texture(uEquirectangularMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}
