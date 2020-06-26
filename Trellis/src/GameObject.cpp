#include "game_object.h"
#include "util.h"

GameObject::GameObject() : transform(),
        Color(1.0f),
        Sprite(),
        Clickable(true) {
    Uid = Util::generate_uid();
}

GameObject::GameObject(
        const Transform &transform,
        Texture2D sprite,
        uint64_t uid,
        bool clickable,
        glm::vec3 color) : transform(transform),
        Color(color),
        Sprite(sprite),
        Clickable(clickable) {
    Uid = uid == 0
          ? Uid = Util::generate_uid()
          : uid;
}

void GameObject::Draw(SpriteRenderer *renderer, int border_pixel_width) {
    renderer->DrawSprite(
            Sprite,
            transform.position,
            border_pixel_width,
            transform.scale,
            transform.rotation,
            Color);
}
