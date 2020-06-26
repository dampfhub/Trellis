#ifndef BOARD_RENDERER_H
#define BOARD_RENDERER_H

#include "renderer.h"

class BoardRenderer : public Renderer {
private:
    unsigned int quad_VAO;
public:
    BoardRenderer(
            const Transform &transform,
            const glm::mat4 &view);

    ~BoardRenderer() override;

    void Draw() override;

};

#endif //BOARD_RENDERER_H
