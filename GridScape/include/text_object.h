#ifndef TEXT_OBJECT_H
#define TEXT_OBJECT_H

#include <string>

#include "text_renderer.h"

class TextObject {
public:
	glm::vec2 Position;
	float Scale;
	glm::vec3 Color;

	std::string Text;

	int Width;
	int Height;

	TextObject();
	TextObject(std::string text, glm::vec2 pos, float scale = 1.0f, 
			   glm::vec3 color = glm::vec3(1.0f));

	void Draw(TextRenderer *renderer);
};

#endif