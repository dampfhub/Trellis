#ifndef BOARD_RENDERER_H
#define BOARD_RENDERER_H

#include "renderer.h"

class BoardRenderer : public Renderer {
private:
    unsigned int quad_VAO;
    float        LineWidth;
    glm::vec3    Color;
    glm::vec3    LineColor;

public:
    glm::ivec2 &CellDims;
    BoardRenderer(
        const Transform &transform,
        const glm::mat4 &view,
        glm::ivec2 &     cell_dims,
        float            line_width = 0.02,
        glm::vec3        color      = glm::vec3(1),
        glm::vec3        line_color = glm::vec3(0));

    ~BoardRenderer() override;

    void Draw() override;
};

#endif // BOARD_RENDERER_H
