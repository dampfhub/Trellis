#ifndef SPRITE_RENDERER_H
#define SPRITE_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.h"
#include "shader.h"


class SpriteRenderer {
public:
    glm::mat4 View;
    // Constructor (inits shaders/shapes)
    SpriteRenderer(Shader shader, int tile_factor = 1);
    // Destructor
    ~SpriteRenderer();
    // Renders a defined quad textured with given sprite
    void DrawSprite(Texture2D texture, glm::vec2 position, 
                    int border_pixel_width,
                    glm::vec2 size = glm::vec2(10.0f, 10.0f), 
                    float rotate = 0.0f, 
                    glm::vec3 color = glm::vec3(1.0f));
    void Resize(int size);
private:
    // Render state
    Shader       shader;
    unsigned int quadVAO;
    // Initializes and configures the quad's buffer and vertex attributes
    void initRenderData(int tile_factor = 1);
};

#endif