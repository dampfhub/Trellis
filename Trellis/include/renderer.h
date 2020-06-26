#ifndef RENDERER_H
#define RENDERER_H

#include <memory>

#include "shader.h"
#include "transform.h"

class Renderer {
protected:
    std::shared_ptr<Shader> shader;
    Transform &transform;
public:
    Renderer(const std::shared_ptr<Shader> &shader, Transform &transform);

    virtual void Draw() = 0;
};

class SRenderer : public Renderer {

};

#endif //RENDERER_H
