#include "text_object.h"

TextObject::TextObject() : 
		Position(glm::vec2(1.0f, 1.0f)), Scale(1.0f),
		Text(""), Color(glm::vec3(1.0f, 1.0f, 1.0f)),
		Width(0),
		Height(0) { }

TextObject::TextObject(std::string text, 
		glm::vec2 pos,
		float scale, 
		glm::vec3 color) : 
		Position(pos), 
		Scale(scale), 
		Text(text), 
		Color(color),
		Width(0),
		Height(0) { }

void TextObject::Draw(TextRenderer* renderer) {
	renderer->RenderText(this->Text,
		this->Position.x,
		this->Position.y,
		this->Scale,
		this->Color);
}
