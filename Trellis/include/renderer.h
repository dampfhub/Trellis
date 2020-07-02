#ifndef RENDERER_H
#define RENDERER_H

#include "shader.h"
#include "texture.h"
#include "transform.h"

#include <memory>

class Renderer {
protected:
    std::reference_wrapper<const Transform> transform;
    std::reference_wrapper<const glm::mat4> view;

    glm::mat4 Model();

public:
    std::shared_ptr<Shader> shader;

    Renderer(std::shared_ptr<Shader> shader, const Transform &transform, const glm::mat4 &view);

    virtual ~Renderer();

    virtual void Draw() = 0;

    void setTransform(const Transform &transform);
    void setView(const glm::mat4 &view);
};

#endif // RENDERER_H
