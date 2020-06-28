#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "texture.h"
#include "renderer.h"
#include "transform.h"
#include "util.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class GameObject: public Util::Serializable<GameObject> {
public:
    Transform transform;
    glm::vec3 Color;
    uint64_t Uid;

    Texture2D Sprite;

    bool Clickable;

    GameObject();
    GameObject(
            const Transform &transform,
            Texture2D sprite,
            uint64_t uid = 0,
            bool clickable = true,
            glm::vec3 color = glm::vec3(1.0f));

    GameObject(const GameObject &) = delete;
    GameObject &operator=(const GameObject &) = delete;

    GameObject(GameObject &&other);
    GameObject &operator=(GameObject &&other);

    void Draw(int border_pixel_width);

    void swap(GameObject &other);
    std::unique_ptr<Renderer> renderer;

    std::vector<std::byte> Serialize() const override;
private:
    friend Serializable<GameObject>;
    static GameObject deserialize(const std::vector<std::byte> &vec);
private:
};

void swap(GameObject &a, GameObject &b);

#endif