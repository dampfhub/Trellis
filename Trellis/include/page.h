#ifndef PAGE_H
#define PAGE_H

#include "camera.h"
#include "game_object.h"
#include "page_ui.h"
#include "util.h"
#include "board_renderer.h"

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

class CorePage : public Util::Serializable<CorePage> {
public:
    uint64_t    Uid{0};
    std::string Name;

    CorePage() = default;
    CorePage(
        std::string       name,
        const Transform & boardTransform = {glm::vec2(0), glm::vec2(2000), 0},
        const glm::ivec2 &cellDims       = glm::ivec2(20, 20),
        uint64_t          uid            = 0);
    std::vector<std::byte> Serialize() const override;

protected:
    Transform  board_transform;
    glm::ivec2 cell_dims = glm::ivec2(20);

    friend Serializable<CorePage>;
    static CorePage deserialize_impl(const std::vector<std::byte> &vec);
};

class Page : public CorePage {
public:
    using page_list_t    = std::list<std::unique_ptr<Page>>;
    using page_list_it_t = std::list<std::unique_ptr<Page>>::iterator;

    enum class MouseHoverType { NONE, N, E, S, W, NE, SE, SW, NW, CENTER };
    enum class ArrowkeyType { RIGHT, LEFT, DOWN, UP };
    enum class MouseHoldType { NONE, PLACING, FOLLOWING, SCALING };

    static constexpr float TILE_DIMENSIONS = 100.0f;

    glm::mat4                 View = glm::mat4(1.0f);
    std::unique_ptr<Camera2D> Camera;
    std::unique_ptr<PageUI>   UserInterface;
    bool                      Snapping = true;

    Page(const CorePage &other);
    Page &operator=(const CorePage &other);

    ~Page();

    Page(const Page &) = delete;
    Page &operator=(const Page &) = delete;
    Page &operator=(Page &&other) noexcept = delete;

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
    GameObject &AddPiece(const CoreGameObject &core_piece);

    void DeletePiece(uint64_t);

    void DeleteCurrentSelection();

    // Begin placing a piece on board, this locks it to the mouse and doesn't place until clicked.
    void BeginPlacePiece(const Transform &transform, Texture2D sprite);

    void Update(glm::ivec2 mouse_pos);

    void Draw();

    MouseHoverType HoverType(glm::ivec2 mouse_pos, GameObject &object);

    MouseHoverType CurrentHoverType(glm::ivec2 mouse_pos);

    // Network related functions
    void SendAllPieces(uint64_t target_uid = 0);

    // If page has an active selection, deselect it and return true. Otherwise,
    // return false. If a new piece is being placed, delete it.
    bool Deselect();

    std::list<std::unique_ptr<GameObject>>                           Pieces;
    std::unordered_map<uint64_t, std::reference_wrapper<GameObject>> PiecesMap;
    std::list<std::unique_ptr<GameObject>>::iterator CurrentSelection = Pieces.end();

    glm::ivec2 getCellDims() const;
    void       setCellDims(glm::ivec2 cellDims);

private:
    BoardRenderer       board_renderer;
    glm::ivec2          DragOrigin = glm::ivec2(0);
    MouseHoldType       mouse_hold = MouseHoldType::NONE;
    std::pair<int, int> ScaleEdges = {0, 0};
    glm::vec2           initialSize;
    glm::vec2           initialPos;
    int                 BorderWidth = 5;

    void SnapPieceToGrid(GameObject &piece, int increments);

    glm::vec2 ScreenPosToWorldPos(glm::ivec2 pos);

    glm::vec2 WorldPosToScreenPos(glm::ivec2 pos);

    void MoveCurrentSelection(glm::vec2 mouse_pos);

    void HandleUIEvents();
};

#endif