#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "texture.h"
#include "sprite_renderer.h"
#include "renderer.h"
#include "transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class GameObject {
public:
    Transform transform;
    glm::vec3 Color;
    uint64_t Uid;

    Texture2D Sprite;

    bool Clickable;

    GameObject();
    GameObject(
            const Transform &transform,
            const glm::mat4 &View,
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
private:
};

void swap(GameObject &a, GameObject &b);

#endif