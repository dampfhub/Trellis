#version 330 core
in vec2 TexCoords;
in vec4 screen_corners;
out vec4 color;

uniform vec2 screenRes;
uniform vec3 bg_color;
uniform float line_width;

void main() {
    vec2 coord = fract(TexCoords.xy);
    vec2 border = 1 - 2 * abs(coord - 0.5);
    float col = 1 - float(border.x < line_width / 2 || border.y < line_width / 2);
    color = vec4(col * bg_color, 1);
}