#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "texture.h"
#include "sprite_renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GameObject {
public:
    glm::vec2 Position, Size;
    glm::vec3 Color;
    float Rotation;
    uint64_t Uid;

    Texture2D Sprite;

    bool Clickable;

    bool FollowMouse = false;
    bool ScaleMouse = false;
    std::pair<int, int> ScaleEdges;
    glm::vec2 initialSize;
    glm::vec2 initialPos;

    GameObject();

    GameObject(
            glm::vec2 pos,
            glm::vec2 size,
            Texture2D sprite,
            uint64_t uid = 0,
            bool clickable = true,
            glm::vec3 color = glm::vec3(1.0f));

    void Draw(SpriteRenderer *renderer, int border_pixel_width);
};

#endif