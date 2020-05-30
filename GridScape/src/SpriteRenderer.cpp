#include "sprite_renderer.h"
#include "resource_manager.h"

SpriteRenderer::SpriteRenderer(Shader shader, int tile_factor) {
    this->shader = shader;
    this->initRenderData(tile_factor);
}

SpriteRenderer::~SpriteRenderer() {
    glDeleteVertexArrays(1, &this->quadVAO);
}

void SpriteRenderer::DrawSprite(Texture2D texture, glm::vec2 position, bool draw_border,
                                glm::vec2 size, float rotate, glm::vec3 color) {
    // prepare transformations
    this->shader.Use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)

    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); // move origin of rotation to center of quad
    model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f)); // then rotate
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f)); // move origin back

    model = glm::scale(model, glm::vec3(size, 1.0f)); // last scale

    this->shader.SetMatrix4("view", this->View);
    this->shader.SetMatrix4("model", model);

    // render textured quad
    this->shader.SetVector3f("spriteColor", color);

    this->shader.SetFloat("aspect", 1.0f);
    if (draw_border) {
        this->shader.SetFloat("border_width", 0.05f);
    } else {
        this->shader.SetFloat("border_width", 0.00f);
    }

    glActiveTexture(GL_TEXTURE0);
    texture.Bind();

    glBindVertexArray(this->quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SpriteRenderer::Resize(int size) {
    this->initRenderData(size);
}

void SpriteRenderer::initRenderData(int tile_factor) {
    // configure VAO/VBO
    unsigned int VBO;
    float tex_coord = 1.0f * tile_factor;
    float vertices[] = {
        // pos      // tex
        0.0f, 1.0f, 0.0f,      tex_coord,
        1.0f, 0.0f, tex_coord, 0.0f,
        0.0f, 0.0f, 0.0f,      0.0f,

        0.0f, 1.0f, 0.0f,      tex_coord,
        1.0f, 1.0f, tex_coord, tex_coord,
        1.0f, 0.0f, tex_coord, 0.0f
    };

    glGenVertexArrays(1, &this->quadVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(this->quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
