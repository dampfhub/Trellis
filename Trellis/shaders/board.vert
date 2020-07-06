#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;
out mat4 inv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform ivec2 num_cells;

void main() {
    TexCoords = vertex.zw * num_cells;
    mat4 mat = projection * view * model;
    inv = inverse(mat);
    gl_Position = mat * vec4(vertex.xy, 0.0, 1.0);
}