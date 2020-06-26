#ifndef PAGE_H
#define PAGE_H

#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

#include "camera.h"
#include "game_object.h"
#include "sprite_renderer.h"
#include "text_renderer.h"
#include "page_ui.h"

class Page {
public:
    enum class MouseHoverType {
        NONE, N, E, S, W, NE, SE, SW, NW, CENTER
    };

    enum class ArrowkeyType {
        RIGHT, LEFT, DOWN, UP
    };

    static constexpr float TILE_DIMENSIONS = 100.0f;

    using page_list_t = std::list<std::unique_ptr<Page>>;
    using page_list_it_t = std::list<std::unique_ptr<Page>>::iterator;

    glm::mat4 View = glm::mat4(1.0f);


    std::string Name;
    Texture2D Board_Texture;
    std::unique_ptr<SpriteRenderer> Renderer;

    glm::vec2 Position, Size;

    std::unique_ptr<Camera2D> Camera;
    std::unique_ptr<PageUI> UserInterface;

    uint64_t Uid;

    ~Page();

    Page(
            std::string name,
            Texture2D board_tex,
            std::unique_ptr<SpriteRenderer> &&renderer,
            glm::vec2 pos = glm::vec2(0.0f, 0.0f),
            glm::vec2 size = glm::vec2(100.0f, 100.0f),
            // TODO: This should be 0 when we are sending pages correctly
            uint64_t uid = 1);

    // Mouse event handlers
    void HandleLeftClickPress(glm::ivec2 mouse_pos);

    void HandleLeftClickHold(glm::ivec2 mouse_pos);

    void HandleLeftClickRelease(glm::ivec2 mouse_pos);

    void HandleRightClick(glm::ivec2 mouse_pos);

    void HandleMiddleClickPress(glm::ivec2 mouse_pos);

    void HandleMiddleClickHold(glm::ivec2 mouse_pos);

    void HandleScrollWheel(glm::ivec2 mouse_pos, int direction);

    // Keyboard event handlers
    void HandleArrows(ArrowkeyType key);

    // Adds a piece to the pieces list and the map
    void AddPiece(std::unique_ptr<GameObject> &&piece);

    // Begin placing a piece on board, this locks it to the mouse and doesn't place until clicked.
    void BeginPlacePiece(std::unique_ptr<GameObject> &&piece);

    void Update(glm::ivec2 mouse_pos);

    void Draw(SpriteRenderer *sprite_renderer, TextRenderer *text_renderer);

    MouseHoverType HoverType(glm::ivec2 mouse_pos, GameObject &object);

    MouseHoverType CurrentHoverType(glm::ivec2 mouse_pos);

    // If page has an active selection, deselect it and return true. Otherwise,
    // return false.
    bool Deselect();

    std::list<std::unique_ptr<GameObject>> Pieces;
    std::unordered_map<uint64_t, std::reference_wrapper<GameObject>> PiecesMap;

private:
    std::list<std::unique_ptr<GameObject>>::iterator CurrentSelection = Pieces.end();
    glm::ivec2 DragOrigin = glm::ivec2(0);
    enum class MouseHoldType {
        NONE, PLACING, FOLLOWING, SCALING
    } mouse_hold;
    std::pair<int, int> ScaleEdges;
    glm::vec2 initialSize;
    glm::vec2 initialPos;
    int BorderWidth = 5;

    void SnapPieceToGrid(GameObject &piece, int increments);

    glm::vec2 ScreenPosToWorldPos(glm::ivec2 pos);

    glm::vec2 WorldPosToScreenPos(glm::ivec2 pos);

    void MoveCurrentSelection(glm::vec2 mouse_pos);

    void HandleUIEvents();
};

#endif