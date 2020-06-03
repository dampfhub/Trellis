#include "game_object.h"

GameObject::GameObject() : Position(0.0f, 0.0f),
        Size(1.0f, 1.0f),
        Color(1.0f),
        Rotation(0.0f),
        Sprite(),
        Clickable(true) {
}

GameObject::GameObject(
        glm::vec2 pos,
        glm::vec2 size,
        Texture2D sprite,
        bool clickable,
        glm::vec3 color) : Position(pos),
        Size(size),
        Color(color),
        Rotation(0.0f),
        Sprite(sprite),
        Clickable(clickable) {
}

void GameObject::Draw(SpriteRenderer *renderer, int border_pixel_width) {
    renderer->DrawSprite(
            Sprite, Position, border_pixel_width, Size, Rotation, Color);
}

bool GameObject::CheckContainment(glm::vec2 pos) {
    if (pos.x >= Position.x && pos.x <= Position.x + Size.x) {
        if (pos.y >= Position.y && pos.y <= Position.y + Size.y) {
            return true;
        }
    }
    return false;
}

glm::vec2 GameObject::DistanceFromTopLeft(glm::vec2 pos) {
    return pos - Position;
}
