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
    Shader shader;
    Renderer(
            const Shader &shader,
            const Transform &transform,
            const glm::mat4 &view);

    virtual ~Renderer();
    virtual void Draw() = 0;
};

class SRenderer : public Renderer {
private:
    Texture2D Sprite;
    unsigned int quad_VAO;
public:
    SRenderer(
            const Transform &transform,
            const glm::mat4 &view,
            const Texture2D &sprite);

    ~SRenderer() override;

    void Draw() override;

};

#endif //RENDERER_H
