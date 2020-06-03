#include "page.h"
#include "resource_manager.h"
#include "text_renderer.h"
#include "text_object.h"
#include "game.h"

Page::Page(
        std::string name,
        Texture2D board_tex,
        SpriteRenderer *renderer,
        glm::vec2 pos,
        glm::vec2 size) : Name(name),
        Board_Texture(board_tex),
        Renderer(renderer),
        Position(pos),
        Size(size) {
    Renderer->Resize((int)Size.x);
    Camera = new Camera2D(
            200.0f, glm::vec2(0.4f, 2.5f));
    UserInterface = new PageUI();
}

Page::~Page() {
    for (GameObject *piece : Pieces) {
        delete piece;
    }
    delete Camera;
}

void Page::PlacePiece(GameObject *piece, bool grid_locked) {
    if (grid_locked) {
        piece->Position = glm::vec2(
                piece->Position.x * TILE_DIMENSIONS + 1,
                piece->Position.y * TILE_DIMENSIONS + 1);
    }
    Pieces.push_back(piece);
}

void Page::BeginPlacePiece(GameObject *piece) {
    Pieces.push_front(piece);
    CurrentSelection = Pieces.begin();
    Placing = true;
}

void Page::Update(float dt) {
    (void)dt;
    HandleUIEvents();
}

void Page::UpdatePlacing(glm::ivec2 mouse_pos) {
    glm::vec2 world_mouse = ScreenPosToWorldPos(mouse_pos);
    (*CurrentSelection)->Position =
            world_mouse - (*CurrentSelection)->Size / 2.0f;
}

void Page::Draw(SpriteRenderer *sprite_renderer, TextRenderer *text_renderer) {
    (void)text_renderer;
    Renderer->View = View;
    sprite_renderer->View = View;
    Renderer->DrawSprite(
            Board_Texture, Position, false, Size * TILE_DIMENSIONS);
    // Draw sprites back-to-front, so the "top" sprite is drawn above the others
    for (auto it = Pieces.rbegin(); it != Pieces.rend(); it++) {
        int border_pixel_width =
                CurrentSelection != Pieces.end() && (*it) == (*CurrentSelection)
                ? BorderWidth
                : 0;
        (*it)->Draw(sprite_renderer, border_pixel_width);
    }
    // Draw user interface
    UserInterface->DrawPieceClickMenu();
}

void Page::HandleUIEvents() {
    if (UserInterface->MoveToFront) {
        Pieces.splice(Pieces.begin(), Pieces, CurrentSelection);
    } else if (UserInterface->MoveToBack) {
        Pieces.splice(Pieces.end(), Pieces, CurrentSelection);
    }
    UserInterface->ClearFlags();
}

MouseHoverType Page::MouseHoverSelection(glm::ivec2 mouse_pos) {
    glm::vec2 world_mouse = ScreenPosToWorldPos(mouse_pos);
    if (CurrentSelection == Pieces.end() ||
            !(*CurrentSelection)->CheckContainment(world_mouse)) {
        return NONE;
    }
    GameObject *piece = *CurrentSelection;
    glm::vec2 click_pos = piece->DistanceFromTopLeft(world_mouse);
    if (click_pos.x <= BorderWidth) {
        if (click_pos.y <= BorderWidth) {
            return NWSE;
        } else if (piece->Size.y - click_pos.y <= BorderWidth) {
            return NESW;
        } else {
            return EW;
        }
    } else if (piece->Size.x - click_pos.x <= BorderWidth) {
        if (click_pos.y <= BorderWidth) {
            return NESW;
        } else if (piece->Size.y - click_pos.y <= BorderWidth) {
            return NWSE;
        } else {
            return EW;
        }
    } else {
        if (click_pos.y <= BorderWidth) {
            return NS;
        } else if (piece->Size.y - click_pos.y <= BorderWidth) {
            return NS;
        } else {
            return CENTER;
        }
    }
}

void Page::HandleLeftClickPress(glm::ivec2 mouse_pos) {
    glm::vec2 world_mouse = ScreenPosToWorldPos(mouse_pos);
    CurrentSelection = Pieces.end();
    // Piece that is currently being placed
    if (Placing) {
        Placing = false;
        return;
    }
    for (auto it = Pieces.begin(); it != Pieces.end(); it++) {
        if ((*it)->CheckContainment(world_mouse) && (*it)->Clickable) {
            glm::vec2 click_pos = (*it)->DistanceFromTopLeft(world_mouse);
            // Check if the user is clicking on any of the sprite's edges /
            // corners
            (*it)->ScaleEdges = std::make_pair(0, 0);
            if (click_pos.x <= BorderWidth) {
                (*it)->ScaleMouse = true;
                (*it)->ScaleEdges.first = -1;
            } else if ((*it)->Size.x - click_pos.x <= BorderWidth) {
                (*it)->ScaleMouse = true;
                (*it)->ScaleEdges.first = 1;
            }
            if (click_pos.y <= BorderWidth) {
                (*it)->ScaleMouse = true;
                (*it)->ScaleEdges.second = -1;
            } else if ((*it)->Size.y - click_pos.y <= BorderWidth) {
                (*it)->ScaleMouse = true;
                (*it)->ScaleEdges.second = 1;
            }
            (*it)->initialSize = (*it)->Size;
            (*it)->initialPos = (*it)->Position;
            // If not scaling, enable dragging instead
            (*it)->FollowMouse = !(*it)->ScaleMouse;
            DragOrigin = click_pos;
            CurrentSelection = it;
            break;
        }
    }
}

void Page::HandleLeftClickHold(glm::ivec2 mouse_pos) {
    static Game &game = Game::GetInstance();
    int inc = game.snapping
              ? 1
              : 8;
    float closest;
    glm::vec2 world_mouse = ScreenPosToWorldPos(mouse_pos);
    if (CurrentSelection != Pieces.end()) {
        GameObject *piece = *CurrentSelection;
        if (piece->FollowMouse) {
            piece->Position = world_mouse - (glm::vec2)DragOrigin;
            SnapPieceToGrid(*CurrentSelection, inc);
        } else if (piece->ScaleMouse) {
            glm::vec2 diff = piece->DistanceFromTopLeft(world_mouse) -
                    (glm::vec2)DragOrigin;
            switch (piece->ScaleEdges.first) {
                case 1:
                    piece->Size.x = fmax(
                            piece->initialSize.x + diff.x,
                            TILE_DIMENSIONS / (float)inc);
                    closest = floor(
                            piece->Size.x / (TILE_DIMENSIONS / (float)inc) +
                                    0.5);
                    piece->Size.x = closest * TILE_DIMENSIONS / (float)inc - 2;
                    break;
                case -1:
                    piece->Size.x = fmax(
                            DragOrigin.x + piece->initialPos.x +
                                    piece->initialSize.x - world_mouse.x,
                            TILE_DIMENSIONS / (float)inc);
                    closest = floor(
                            piece->Size.x / (TILE_DIMENSIONS / (float)inc) +
                                    0.5);
                    piece->Size.x = closest * TILE_DIMENSIONS / (float)inc - 2;
                    piece->Position.x =
                            piece->initialPos.x + piece->initialSize.x -
                                    piece->Size.x;
                    break;
            }
            switch (piece->ScaleEdges.second) {
                case 1:
                    piece->Size.y = fmax(
                            piece->initialSize.y + diff.y,
                            TILE_DIMENSIONS / (float)inc);
                    closest = floor(
                            piece->Size.y / (TILE_DIMENSIONS / (float)inc) +
                                    0.5);
                    piece->Size.y = closest * TILE_DIMENSIONS / (float)inc - 2;
                    break;
                case -1:
                    piece->Size.y = fmax(
                            DragOrigin.y + piece->initialPos.y +
                                    piece->initialSize.y - world_mouse.y,
                            TILE_DIMENSIONS / (float)inc);
                    closest = floor(
                            piece->Size.y / (TILE_DIMENSIONS / (float)inc) +
                                    0.5);
                    piece->Size.y = closest * TILE_DIMENSIONS / (float)inc - 2;
                    piece->Position.y =
                            piece->initialPos.y + piece->initialSize.y -
                                    piece->Size.y;
                    break;
            }
        }
    }
}

void Page::HandleLeftClickRelease(glm::ivec2 mouse_pos) {
    (void)mouse_pos;
    if (CurrentSelection != Pieces.end()) {
        (*CurrentSelection)->FollowMouse = false;
        (*CurrentSelection)->ScaleMouse = false;
    }
}

void Page::HandleRightClick(glm::ivec2 mouse_pos) {
    glm::vec2 world_mouse = ScreenPosToWorldPos(mouse_pos);
    if (CurrentSelection != Pieces.end()) {
        if ((*CurrentSelection)->CheckContainment(world_mouse)) {
            UserInterface->ClickMenuActive = true;
        } else {
            CurrentSelection = Pieces.end();
        }
    }
}

void Page::HandleMiddleClickPress(glm::ivec2 mouse_pos) {
    DragOrigin = mouse_pos;
}

void Page::HandleMiddleClickHold(glm::ivec2 mouse_pos) {
    glm::vec2 v = mouse_pos - DragOrigin;
    Camera->Move(v);
    View = Camera->CalculateView(Size * TILE_DIMENSIONS);
    DragOrigin = mouse_pos;
}

void Page::HandleScrollWheel(glm::ivec2 mouse_pos, int scroll_direction) {
    Camera->Zoom(mouse_pos, scroll_direction);
    View = Camera->CalculateView(Size * TILE_DIMENSIONS);
}

void Page::SnapPieceToGrid(GameObject *piece, int increments) {
    float closest_x = (float)floor(
            piece->Position.x / (TILE_DIMENSIONS / (float)increments) + 0.5);
    float closest_y = (float)floor(
            piece->Position.y / (TILE_DIMENSIONS / (float)increments) + 0.5);
    piece->Position = glm::vec2(
            closest_x * TILE_DIMENSIONS / (float)increments + 1,
            closest_y * TILE_DIMENSIONS / (float)increments + 1);
}

glm::vec2 Page::ScreenPosToWorldPos(glm::ivec2 pos) {
    glm::vec4 world_pos = glm::inverse(View) * glm::vec4(pos, 0.0f, 1.0f);
    return glm::vec2(world_pos.x, world_pos.y);
}
