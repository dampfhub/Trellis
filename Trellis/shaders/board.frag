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

void main() {
    vec2 coord = TexCoords.xy;
    vec2 fs = ((gl_FragCoord.xy + vec2(1))/ screenRes) * 2 - 1;
    vec2 mf = (inv * vec4(fs.xy, 0, 1)).xy * num_cells;
    vec2 c1 = fract(coord);
    vec2 c2 = fract(mf);

    vec2 d = mf - coord;

    vec2 f1 = floor(coord);
    vec2 f2 = floor(mf);

    color = vec4(1);
    float d2 = c2.x - c1.x;
    if (f1.x != f2.x) {
        d2 = 1 - c1.x + c2.x;
        color.rgb = vec3(1 - line_width / d2);
        return;
    }
    if (mf.x - coord.x < line_width / 2) {
        if (c1.x <= line_width/4 || c1.x >= 1 - line_width/4) {
            color.rgb = vec3(0);
        }
        return;
    }

    if (c1.x <= line_width/4) {
        if (c2.x <= line_width/4) {
            color.rgb = vec3(0);
        } else {
            color.rgb = vec3((c2.x - line_width/4) / d2);
        }
    } else if (c1.x >= 1 - line_width/4) {
        color.rgb = vec3(0);
    }

    //if (c1.x >= 1 - line_width/4 || c1.x <= line_width/4) {
    //    color.rgb = vec3((mf.x - coord.x) / line_width );
    //}



    /*
    if (c1.x < 1 && c2.x > 1 - line_width) {
        float a = 1 - line_width;
        if (c1.x < 1 - line_width && c2.x > 1 - line_width) {
            float f = (a - c1.x) / (c2.x - c1.x);
            color.xyz = vec3(f);
        } else if (c2.x > 1 && c1.x < 1) {
            float f = (1 - c1.x) / (c2.x - c1.x);
            color.xyz = vec3(f);
        } else {
            color.xyz = vec3(0);
        }
    }
    */
}