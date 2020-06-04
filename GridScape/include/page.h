#ifndef PAGE_H
#define PAGE_H

#include <list>
#include <memory>
#include <string>

#include "camera.h"
#include "game_object.h"
#include "sprite_renderer.h"
#include "text_renderer.h"
#include "page_ui.h"

enum MouseHoverType {
    NONE, N, E, S, W, NE, SE, SW, NW, CENTER
};

class Page {
public:
    static constexpr float TILE_DIMENSIONS = 100.0f;

    glm::mat4 View = glm::mat4(1.0f);

    bool Placing = false;

    std::string Name;
    Texture2D Board_Texture;
    SpriteRenderer *Renderer;

    glm::vec2 Position, Size;

    Camera2D *Camera;
    PageUI *UserInterface;

    ~Page();

    Page(
            std::string name,
            Texture2D board_tex,
            SpriteRenderer *renderer,
            glm::vec2 pos = glm::vec2(0.0f, 0.0f),
            glm::vec2 size = glm::vec2(100.0f, 100.0f));

    // Mouse event handlers
    void HandleLeftClickPress(glm::ivec2 mouse_pos);

    void HandleLeftClickHold(glm::ivec2 mouse_pos);

    void HandleLeftClickRelease(glm::ivec2 mouse_pos);

    void HandleRightClick(glm::ivec2 mouse_pos);

    void HandleMiddleClickPress(glm::ivec2 mouse_pos);

    void HandleMiddleClickHold(glm::ivec2 mouse_pos);

    void HandleScrollWheel(glm::ivec2 mouse_pos, int direction);

    // Place piece on board, optionally grid locked
    void PlacePiece(GameObject *piece, bool grid_locked = true);

    // Begin placing a piece on board, this locks it to the mouse and doesn't place until clicked.
    void BeginPlacePiece(GameObject *piece);

    void Update(float dt);

    // If placing a piece, call this
    void UpdatePlacing(glm::ivec2 mouse_pos);

    void Draw(SpriteRenderer *sprite_renderer, TextRenderer *text_renderer);

    MouseHoverType HoverType(glm::ivec2 mouse_pos, GameObject *object);
    MouseHoverType CurrentHoverType(glm::ivec2 mouse_pos);

private:
    std::list<GameObject *> Pieces;
    std::list<GameObject *>::iterator CurrentSelection = Pieces.end();
    glm::ivec2 DragOrigin = glm::ivec2(0);
    int BorderWidth = 5;

    void SnapPieceToGrid(GameObject *piece, int increments);

    glm::vec2 ScreenPosToWorldPos(glm::ivec2 pos);
    glm::vec2 WorldPosToScreenPos(glm::ivec2 pos);

    void MoveCurrentSelection(glm::vec2 mouse_pos);

    void HandleUIEvents();
};

#endif