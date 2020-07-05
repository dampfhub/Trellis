#ifndef SPRITE_RENDERER_H
#define SPRITE_RENDERER_H

#include "renderer.h"

class SpriteRenderer : public Renderer {
private:
    std::reference_wrapper<std::shared_ptr<Texture2D>> Sprite;
    unsigned int                                       quad_VAO;

public:
    SpriteRenderer(
        const Transform &                 transform,
        const glm::mat4 &                 view,
        std::shared_ptr<Texture2D> &sprite);

    ~SpriteRenderer() override;

    void Draw() override;
};

#endif // SPRITE_RENDERER_H
