#include "game_object.h"
#include "util.h"
#include "sprite_renderer.h"

using std::move, std::exchange, std::make_unique;

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

void GameObject::Draw(int border_pixel_width) {
    this->renderer->shader->SetInteger("border_width", border_pixel_width);
    this->renderer->Draw();
}

GameObject::GameObject(GameObject &&other) noexcept: transform(other.transform),
        Color(other.Color),
        Uid(exchange(other.Uid, -1)),
        Sprite(other.Sprite),
        Clickable(other.Clickable),
        renderer(exchange(other.renderer, nullptr)) {
}

GameObject &GameObject::operator=(GameObject &&other) noexcept {
    GameObject copy(move(other));
    copy.swap(*this);
    return *this;
}

void GameObject::swap(GameObject &other) {
    using std::swap;
    swap(transform, other.transform);
    swap(Color, other.Color);
    swap(Uid, other.Uid);
    swap(Sprite, other.Sprite);
    swap(Clickable, other.Clickable);
    swap(renderer, other.renderer);
}

void swap(GameObject &a, GameObject &b) {
    a.swap(b);
}
