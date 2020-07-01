#include "game_object.h"
#include "sprite_renderer.h"
#include "util.h"

using std::move, std::exchange, std::make_unique, std::vector, std::byte;

CoreGameObject::CoreGameObject(
    const Transform &transform,
    uint64_t         sprite_uid,
    uint64_t         uid,
    bool             clickable,
    glm::vec3        color)
    : transform(transform)
    , Color(color)
    , Uid(uid)
    , Clickable(clickable)
    , SpriteUid(sprite_uid) {}

vector<byte>
CoreGameObject::Serialize() const {
    std::vector<std::vector<byte>> bytes;
    bytes.push_back(Util::serialize_vec(transform));
    bytes.push_back(Util::serialize_vec(Color));
    bytes.push_back(Util::serialize_vec(Uid));
    bytes.push_back(Util::serialize_vec(Clickable));
    bytes.push_back(Util::serialize_vec(SpriteUid));
    return Util::flatten(bytes);
}

CoreGameObject
CoreGameObject::deserialize_impl(const vector<byte> &vec) {
    CoreGameObject      g;
    const byte *        ptr = vec.data();
    g.transform             = Util::deserialize<Transform>(ptr);
    g.Color                 = Util::deserialize<glm::vec3>(ptr += sizeof(g.transform));
    g.Uid                   = Util::deserialize<uint64_t>(ptr += sizeof(g.Color));
    g.Clickable             = Util::deserialize<bool>(ptr += sizeof(g.Uid));
    g.SpriteUid             = Util::deserialize<uint64_t>(ptr += sizeof(g.Clickable));
    return g;
}

GameObject &
GameObject::operator=(const CoreGameObject other) {
    (CoreGameObject &)*this = other;
    return *this;
}

GameObject::GameObject(CoreGameObject other) {
    (CoreGameObject &)*this = other;
}

GameObject::GameObject() {
    Uid = Util::generate_uid();
}

GameObject::GameObject(
    const Transform &transform,
    Texture2D        sprite,
    uint64_t         uid,
    bool             clickable,
    glm::vec3        color)
    : Sprite(sprite)
    , CoreGameObject(transform, sprite.ImageUID, uid, clickable, color) {
    Uid = uid == 0 ? Uid = Util::generate_uid() : uid;
}

void
GameObject::Draw(int border_pixel_width) {
    this->renderer->shader->SetInteger("border_width", border_pixel_width);
    this->renderer->Draw();
}

GameObject::GameObject(GameObject &&other) noexcept
    : Sprite(other.Sprite)
    , renderer(exchange(other.renderer, nullptr)) {
    transform = other.transform;
    Color     = other.Color;
    Uid       = exchange(other.Uid, -1);
    Clickable = other.Clickable;
}

GameObject &
GameObject::operator=(GameObject &&other) noexcept {
    GameObject copy(move(other));
    copy.swap(*this);
    return *this;
}

void
GameObject::swap(GameObject &other) {
    using std::swap;
    swap(transform, other.transform);
    swap(Color, other.Color);
    swap(Uid, other.Uid);
    swap(Sprite, other.Sprite);
    swap(Clickable, other.Clickable);
    swap(renderer, other.renderer);
}

void
swap(GameObject &a, GameObject &b) {
    a.swap(b);
}
