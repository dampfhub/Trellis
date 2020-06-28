#ifndef SPRITE_RENDERER_H
#define SPRITE_RENDERER_H

#include "renderer.h"

class SpriteRenderer : public Renderer {
private:
    std::reference_wrapper<const Texture2D> Sprite;
    unsigned int                            quad_VAO;

public:
    SpriteRenderer(const Transform &transform, const glm::mat4 &view, const Texture2D &sprite);

    ~SpriteRenderer() override;

    void Draw() override;
    void setSprite(const Texture2D &sprite);
};

#endif // SPRITE_RENDERER_H
