#include "text_object.h"

TextObject::TextObject() : Text(""),
        Position(glm::vec2(1.0f, 1.0f)),
        Scale(1.0f),
        Color(glm::vec3(1.0f, 1.0f, 1.0f)),
        Width(0),
        Height(0) {
}

TextObject::TextObject(
        std::string text, glm::vec2 pos, float scale, glm::vec3 color) : Text(
        std::move(text)),
        Position(pos),
        Scale(scale),
        Color(color),
        Width(0),
        Height(0) {
}

void TextObject::Draw(TextRenderer *renderer) {
    renderer->RenderText(
            Text, Position.x, Position.y, Scale, Color);
}
