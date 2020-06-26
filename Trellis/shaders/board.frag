#version 330 core
in vec2 TexCoords;
in vec4 screen_corners;
out vec4 color;

uniform vec2 screenRes;

void main() {
    vec2 coord = fract(TexCoords.xy);
    vec2 border = 1 - 2 * abs(coord - 0.5);
    vec3 col = 1 - vec3(border.x < 0.02 || border.y < 0.02);
    color = vec4(col, 1);
}