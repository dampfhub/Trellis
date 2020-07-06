#version 330 core
in vec2 TexCoords;
in vec4 screen_corners;
in mat4 mat;
in mat4 inv;
in vec4 world;
out vec4 color;

uniform ivec2 num_cells;
uniform vec2 screenRes;
uniform vec3 bg_color;
uniform float line_width;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

float line(float a, float b) {
    float c1 = fract(a);
    float c2 = fract(b);
    int f1 = int(floor(a));
    int f2 = int(floor(b));

    bool b1 = c1 <= line_width/4 || c1 >= 1 - line_width/4;
    bool b2 = c2 <= line_width/4 || c2 >= 1 - line_width/4;
    float d;
    if (f1 != f2) {
        d = (1 - c1) + c2;
        if (abs(f1-f2) > 1 || d > 1) {
            return 1;
        }
        if (b1) {
            if (b2) {
                return 1;
            }
            float x = (1 - c1) + line_width/4;
            return x/d;
        }
        if (b2) {
            float x = line_width/4 + c2;
            return x/d;
        }
        float x = line_width/2;
        return x/d;
    }
    d = c2 - c1;
    if (d > 1) {
        return 1;
    }
    if (b1) {
        if (b2) {
            return 1;
        } else {
            float x = line_width/4 - c1;
            return x/d;
        }
    }
    if (b2) {
        float x = c2 + line_width/4 - 1;
        return x/d;
    }
    return 0;
}

void main() {
    vec2 coord = TexCoords.xy;
    vec2 fs = (vec2(gl_FragCoord.x + 1, gl_FragCoord.y - 1)/ screenRes) * 2 - 1;
    vec2 mf = (inv * vec4(fs.xy, 0, 1)).xy * num_cells;

    float lx = line(coord.x, mf.x);
    float ly = line(coord.y, mf.y);
    color = vec4(1 - vec3(max(lx, ly)), 1);
}