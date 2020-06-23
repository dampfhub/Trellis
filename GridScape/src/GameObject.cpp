#include "game_object.h"
#include "util.h"

GameObject::GameObject() : Position(0.0f, 0.0f),
        Size(1.0f, 1.0f),
        Color(1.0f),
        Rotation(0.0f),
        Sprite(),
        Clickable(true) {
    Uid = Util::generate_uid();
}

GameObject::GameObject(
        glm::vec2 pos,
        glm::vec2 size,
        Texture2D sprite,
        uint64_t uid,
        bool clickable,
        glm::vec3 color) : Position(pos),
        Size(size),
        Color(color),
        Rotation(0.0f),
        Sprite(sprite),
        Clickable(clickable) {
    Uid = uid == 0
          ? Uid = Util::generate_uid()
          : uid;
}

void GameObject::Draw(SpriteRenderer *renderer, int border_pixel_width) {
    renderer->DrawSprite(
            Sprite, Position, border_pixel_width, Size, Rotation, Color);
}

glm::vec2 GameObject::DistanceFromTopLeft(glm::vec2 pos) {
    return pos - Position;
}
