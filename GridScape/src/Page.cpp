#include "page.h"
#include "resource_manager.h"
#include "text_renderer.h"
#include "text_object.h"
#include "glfw_handler.h"
#include <iostream>

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
    this->Renderer->Resize((int)this->Size.x);
    this->Camera = new Camera2D(
            200.0f,
            glm::vec2(
                    this->Size.x * this->TILE_DIMENSIONS,
                    this->Size.y * this->TILE_DIMENSIONS),
            glm::vec2(0.4f, 2.5f));
    this->UserInterface = new PageUI();
}

Page::~Page() {
    for (GameObject *piece : this->Pieces) {
        delete piece;
    }
    delete this->Camera;
}

void Page::PlacePiece(GameObject *piece, bool grid_locked) {
    if (grid_locked) {
        piece->Position = glm::vec2(
                piece->Position.x * this->TILE_DIMENSIONS + 1,
                piece->Position.y * this->TILE_DIMENSIONS + 1);
    }
    this->Pieces.push_back(piece);
}

void Page::BeginPlacePiece(GameObject *piece) {
    Pieces.push_front(piece);
    this->CurrentSelection = Pieces.begin();
    this->Placing = true;
}

void Page::Update(float dt) {
    this->HandleUIEvents();
}

void Page::UpdatePlacing(glm::ivec2 mouse_pos) {
    glm::vec2 world_mouse = this->ScreenPosToWorldPos(mouse_pos);
    (*CurrentSelection)->Position =
            world_mouse - (*CurrentSelection)->Size / 2.0f;
}

void Page::Draw(SpriteRenderer *sprite_renderer, TextRenderer *text_renderer) {
    this->Renderer->View = this->View;
    sprite_renderer->View = this->View;
    this->Renderer->DrawSprite(
            this->Board_Texture,
            this->Position,
            false,
            this->Size * this->TILE_DIMENSIONS);
    // Draw sprites back-to-front, so the "top" sprite is drawn above the others
    for (auto it = Pieces.rbegin(); it != Pieces.rend(); it++) {
        int border_pixel_width =
                CurrentSelection != Pieces.end() && (*it) == (*CurrentSelection)
                ? this->BorderWidth
                : 0;
        (*it)->Draw(sprite_renderer, border_pixel_width);
    }
    // Draw user interface
    this->UserInterface->DrawPieceClickMenu();
}

void Page::HandleUIEvents() {
    if (this->UserInterface->MoveToFront) {
        Pieces.splice(Pieces.begin(), Pieces, CurrentSelection);
    } else if (this->UserInterface->MoveToBack) {
        Pieces.splice(Pieces.end(), Pieces, CurrentSelection);
    }
    this->UserInterface->ClearFlags();
}

MouseHoverType Page::MouseHoverSelection(glm::ivec2 mouse_pos) {
    glm::vec2 world_mouse = this->ScreenPosToWorldPos(mouse_pos);
    if (CurrentSelection == Pieces.end() ||
            !(*CurrentSelection)->CheckContainment(world_mouse)) {
        return NONE;
    }
    GameObject *piece = *CurrentSelection;
    glm::vec2 click_pos = piece->DistanceFromTopLeft(world_mouse);
    if (click_pos.x <= this->BorderWidth) {
        if (click_pos.y <= this->BorderWidth) {
            return NWSE;
        } else if (piece->Size.y - click_pos.y <= this->BorderWidth) {
            return NESW;
        } else {
            return EW;
        }
    } else if (piece->Size.x - click_pos.x <= this->BorderWidth) {
        if (click_pos.y <= this->BorderWidth) {
            return NESW;
        } else if (piece->Size.y - click_pos.y <= this->BorderWidth) {
            return NWSE;
        } else {
            return EW;
        }
    } else {
        if (click_pos.y <= this->BorderWidth) {
            return NS;
        } else if (piece->Size.y - click_pos.y <= this->BorderWidth) {
            return NS;
        } else {
            return CENTER;
        }
    }
}

void Page::HandleLeftClickPress(glm::ivec2 mouse_pos) {
    glm::vec2 world_mouse = this->ScreenPosToWorldPos(mouse_pos);
    CurrentSelection = Pieces.end();
    // Piece that is currently being placed
    if (this->Placing) {
        this->Placing = false;
        return;
    }
    for (auto it = Pieces.begin(); it != Pieces.end(); it++) {
        if ((*it)->CheckContainment(world_mouse) && (*it)->Clickable) {
            glm::vec2 click_pos = (*it)->DistanceFromTopLeft(world_mouse);
            // Check if the user is clicking on any of the sprite's edges /
            // corners
            (*it)->ScaleEdges = std::make_pair(0, 0);
            if (click_pos.x <= this->BorderWidth) {
                (*it)->ScaleMouse = true;
                (*it)->ScaleEdges.first = -1;
            } else if ((*it)->Size.x - click_pos.x <= this->BorderWidth) {
                (*it)->ScaleMouse = true;
                (*it)->ScaleEdges.first = 1;
            }
            if (click_pos.y <= this->BorderWidth) {
                (*it)->ScaleMouse = true;
                (*it)->ScaleEdges.second = -1;
            } else if ((*it)->Size.y - click_pos.y <= this->BorderWidth) {
                (*it)->ScaleMouse = true;
                (*it)->ScaleEdges.second = 1;
            }
            (*it)->initialSize = (*it)->Size;
            (*it)->initialPos = (*it)->Position;
            // If not scaling, enable dragging instead
            (*it)->FollowMouse = !(*it)->ScaleMouse;
            this->DragOrigin = click_pos;
            this->CurrentSelection = it;
            break;
        }
    }
}

void Page::HandleLeftClickHold(glm::ivec2 mouse_pos) {
    glm::vec2 world_mouse = this->ScreenPosToWorldPos(mouse_pos);
    if (CurrentSelection != Pieces.end()) {
        GameObject *piece = *CurrentSelection;
        if (piece->FollowMouse) {
            piece->Position = world_mouse - (glm::vec2)this->DragOrigin;
        } else if (piece->ScaleMouse) {
            glm::vec2 diff = piece->DistanceFromTopLeft(world_mouse) -
                    (glm::vec2)this->DragOrigin;
            switch (piece->ScaleEdges.first) {
                case 1:
                    piece->Size.x = fmax(
                            piece->initialSize.x + diff.x,
                            2 * this->BorderWidth + 1);
                    break;
                case -1:
                    piece->Position.x = fmin(
                            world_mouse.x - this->DragOrigin.x,
                            piece->initialPos.x + piece->initialSize.x -
                                    2 * this->BorderWidth - 1);
                    piece->Size.x = piece->initialSize.x + piece->initialPos.x -
                            piece->Position.x;
                    break;
            }
            switch (piece->ScaleEdges.second) {
                case 1:
                    piece->Size.y = fmax(
                            piece->initialSize.y + diff.y,
                            2 * this->BorderWidth + 1);
                    break;
                case -1:
                    piece->Position.y = fmin(
                            world_mouse.y - this->DragOrigin.y,
                            piece->initialPos.y + piece->initialSize.y -
                                    2 * this->BorderWidth - 1);
                    piece->Size.y = piece->initialSize.y + piece->initialPos.y -
                            piece->Position.y;
                    break;
            }
        }
    }
}

void Page::HandleLeftClickRelease(glm::ivec2 mouse_pos) {
    if (CurrentSelection != Pieces.end()) {
        (*CurrentSelection)->FollowMouse = false;
        (*CurrentSelection)->ScaleMouse = false;
    }
}

void Page::HandleRightClick(glm::ivec2 mouse_pos) {
    glm::vec2 world_mouse = this->ScreenPosToWorldPos(mouse_pos);
    if (CurrentSelection != Pieces.end()) {
        if ((*CurrentSelection)->CheckContainment(world_mouse)) {
            UserInterface->ClickMenuActive = true;
        } else {
            CurrentSelection = Pieces.end();
        }
    }
}

void Page::HandleMiddleClickPress(glm::ivec2 mouse_pos) {
    this->DragOrigin = mouse_pos;
}

void Page::HandleMiddleClickHold(glm::ivec2 mouse_pos) {
    glm::vec2 v = mouse_pos - DragOrigin;
    this->Camera->Move(v);
    this->View =
            this->Camera->CalculateView(this->Size * this->TILE_DIMENSIONS);
    this->DragOrigin = mouse_pos;
}

void Page::HandleScrollWheel(glm::ivec2 mouse_pos, int scroll_direction) {
    this->Camera->Zoom(mouse_pos, scroll_direction);
    this->View =
            this->Camera->CalculateView(this->Size * this->TILE_DIMENSIONS);
}

void Page::SnapPieceToGrid(GameObject *piece) {
    float closest_x =
            (float)floor(piece->Position.x / this->TILE_DIMENSIONS + 0.5);
    float closest_y = (float)floor(
            piece->Position.y / this->TILE_DIMENSIONS + 0.5);
    if (closest_x > this->Size.x) {
        closest_x = this->Size.x - 1;
    }
    if (closest_x < 0) {
        closest_x = 0;
    }
    if (closest_y > this->Size.y) {
        closest_x = this->Size.y - 1;
    }
    if (closest_y < 0) {
        closest_y = 0;
    }
    piece->Position = glm::vec2(
            closest_x * this->TILE_DIMENSIONS + 1,
            closest_y * this->TILE_DIMENSIONS + 1);
}

glm::vec2 Page::ScreenPosToWorldPos(glm::ivec2 pos) {
    glm::vec4 world_pos = glm::inverse(this->View) * glm::vec4(pos, 0.0f, 1.0f);
    return glm::vec2(world_pos.x, world_pos.y);
}
