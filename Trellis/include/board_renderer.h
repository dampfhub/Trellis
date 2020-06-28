#ifndef BOARD_RENDERER_H
#define BOARD_RENDERER_H

#include "renderer.h"

class BoardRenderer : public Renderer {
private:
    unsigned int quad_VAO;
    float        LineWidth;
    glm::vec3    Color;

public:
    glm::ivec2 CellDims;

    BoardRenderer(
        const Transform &transform,
        const glm::mat4 &view,
        float            line_width = 0.04,
        glm::vec3        color      = glm::vec3(1),
        glm::ivec2       cell_dims  = glm::ivec2(20, 20));

    ~BoardRenderer() override;

    void Draw() override;
};

#endif // BOARD_RENDERER_H
