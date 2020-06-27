#ifndef RENDERER_H
#define RENDERER_H

#include <memory>

#include "shader.h"
#include "transform.h"
#include "texture.h"

class Renderer {
protected:
    const Transform &transform;
    const glm::mat4 &view;

    glm::mat4 Model();

public:
    std::shared_ptr<Shader> shader;
    Renderer(
            const std::shared_ptr<Shader> &shader,
            const Transform &transform,
            const glm::mat4 &view);

    virtual ~Renderer();
    virtual void Draw() = 0;
};

#endif //RENDERER_H