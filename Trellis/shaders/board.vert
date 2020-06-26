#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;
out vec4 screen_corners;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 screenRes;

void main() {
    TexCoords = vertex.zw * 20;
    mat4 mat = projection * view * model;
    gl_Position = mat * vec4(vertex.xy, 0.0, 1.0);
    screen_corners = vec4(screenRes.xy, screenRes.xy) * (vec4(
       (mat * vec4(0, 1, 0, 1)).xy,
       (mat * vec4(1, 0, 0, 1)).xy
    ) + 1)/2;
}