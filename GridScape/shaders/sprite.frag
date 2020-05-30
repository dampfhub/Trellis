#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor;
uniform float border_width;
uniform float aspect;  // ratio of width to height

void main() {    
   float maxX = 1.0 - border_width;
   float minX = border_width;
   float maxY = maxX / aspect;
   float minY = minX / aspect;

	if (border_width == 0) {
		color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
		return;
	}

   if (TexCoords.x <= maxX && TexCoords.x >= minX &&
       TexCoords.y <= maxY && TexCoords.y >= minY) {
        color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
    } else {
        color = vec4(0.0, 0.5, 0.5, 1.0);
    }
}  