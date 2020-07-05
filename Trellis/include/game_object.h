#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "renderer.h"
#include "texture.h"
#include "transform.h"
#include "util.h"
#include "sqlite_handler.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class CoreGameObject : public Util::Serializable<CoreGameObject> {
public:
    Transform transform;
    glm::vec3 Color{};
    uint64_t  Uid{};
    uint64_t  SpriteUid{};
    bool      Clickable{};

    CoreGameObject() = default;
    CoreGameObject(
        const Transform &transform,
        uint64_t         sprite_uid,
        uint64_t         uid,
        bool             clickable,
        glm::vec3        color);
    CoreGameObject(const SQLite::Database &db, uint64_t uid);

    std::vector<std::byte> Serialize() const override;

private:
    friend Serializable<CoreGameObject>;
    static CoreGameObject deserialize_impl(const std::vector<std::byte> &vec);
};

class GameObject : public CoreGameObject {
public:
    GameObject(const CoreGameObject &other, glm::mat4 &View);
    GameObject(const GameObject &) = delete;
    GameObject &operator=(const GameObject &) = delete;

    GameObject(GameObject &&other) noexcept;
    GameObject &operator=(GameObject &&other) noexcept;

    GameObject &operator=(const CoreGameObject &other);

    void Draw(int border_pixel_width);
    void WriteToDB(const SQLite::Database &db, uint64_t page_id) const;

    void UpdateSprite(uint64_t sprite_uid);

private:
    explicit GameObject(const CoreGameObject &other);
    void swap(GameObject &other);

    Texture2D                 Sprite;
    std::unique_ptr<Renderer> renderer;
};

void swap(GameObject &a, GameObject &b);

#endif