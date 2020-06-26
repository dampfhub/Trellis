#ifndef BOARD_RENDERER_H
#define BOARD_RENDERER_H

#include "renderer.h"

class BoardRenderer : public Renderer {
private:
    Texture2D Sprite;
    unsigned int quad_VAO;
public:
    BoardRenderer(
            const Transform &transform,
            const glm::mat4 &view,
            const Texture2D &sprite);

    ~BoardRenderer() override;

    void Draw() override;

};

#endif //BOARD_RENDERER_H
