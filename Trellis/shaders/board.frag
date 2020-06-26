#version 330 core
in vec2 TexCoords;
in vec4 screen_corners;
out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor;
uniform int border_width;
uniform vec2 screenRes;

void main() {
	if (border_width == 0) {
		color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
		return;
	}
    if (abs(gl_FragCoord.x - screen_corners.x) < border_width ||
        abs(gl_FragCoord.y - screen_corners.y) < border_width ||
        abs(screen_corners.z - gl_FragCoord.x) < border_width ||
        abs(screen_corners.w - gl_FragCoord.y) < border_width) {
        color = vec4(0.0, 0.5, 0.5, 1.0);
        return;
    }
    color = vec4(spriteColor, 1.0) * texture(image, TexCoords);

}  