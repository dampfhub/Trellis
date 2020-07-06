#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;
out vec4 screen_corners;
out mat4 mat;
out mat4 inv;
out vec4 world;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 screenRes;
uniform ivec2 num_cells;

void main() {
    TexCoords = vertex.zw * num_cells;
    mat = projection * view * model;
    inv = inverse(mat);
    gl_Position = mat * vec4(vertex.xy, 0.0, 1.0);
    world = model * vec4(vertex.xy, 0.0, 1.0);
}