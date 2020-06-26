#include "page.h"
#include "resource_manager.h"
#include "text_renderer.h"
#include "text_object.h"
#include "game.h"
#include "util.h"
#include "client_server.h"

Page::Page(
        std::string name,
        Texture2D board_tex,
        std::unique_ptr <SpriteRenderer> &&renderer,
        glm::vec2 pos,
        glm::vec2 size,
        uint64_t uid) : Name(name),
        Board_Texture(board_tex),
        Renderer(std::move(renderer)),
        Position(pos),
        Size(size),
        Uid(uid) {
    Renderer->Resize((int)Size.x);
    Camera = std::make_unique<Camera2D>(
            200.0f, glm::vec2(0.4f, 2.5f));
    UserInterface = std::make_unique<PageUI>();
    if (Uid == 0) {
        Uid = Util::generate_uid();
    }
}

Page::~Page() {
}

void Page::AddPiece(std::unique_ptr<GameObject> &&piece) {
    PiecesMap.insert(std::make_pair(piece->Uid, std::ref(*piece)));
    Pieces.push_front(std::move(piece));
}

void Page::BeginPlacePiece(std::unique_ptr <GameObject> &&piece) {
    Placing = true;
    piece->FollowMouse = true;
    piece->initialSize = piece->Size;
    piece->initialPos = piece->Position;
    AddPiece(std::move(piece));
    CurrentSelection = Pieces.begin();
}

void Page::Update(float dt) {
    (void)dt;
    HandleUIEvents();
}

void Page::UpdatePlacing(glm::ivec2 mouse_pos) {
    MoveCurrentSelection(mouse_pos);
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

MouseHoverType Page::HoverType(
        glm::ivec2 mouse_pos, GameObject &object) {
    glm::ivec2 NW_corner_screen = WorldPosToScreenPos(object.Position);
    glm::ivec2 SE_corner_screen =
            WorldPosToScreenPos(object.Position + object.Size);
    if (mouse_pos.x < NW_corner_screen.x || mouse_pos.y < NW_corner_screen.y ||
            mouse_pos.x > SE_corner_screen.x ||
            mouse_pos.y > SE_corner_screen.y) {
        return NONE;
    }
    glm::ivec2 size_screen = SE_corner_screen - NW_corner_screen;
    glm::ivec2 click_pos = mouse_pos - (glm::ivec2)NW_corner_screen;
    if (click_pos.x <= BorderWidth) {
        if (click_pos.y <= BorderWidth) {
            return NW;
        } else if (size_screen.y - click_pos.y <= BorderWidth) {
            return SW;
        } else {
            return W;
        }
    } else if (size_screen.x - click_pos.x <= BorderWidth) {
        if (click_pos.y <= BorderWidth) {
            return NE;
        } else if (size_screen.y - click_pos.y <= BorderWidth) {
            return SE;
        } else {
            return E;
        }
    } else {
        if (click_pos.y <= BorderWidth) {
            return N;
        } else if (size_screen.y - click_pos.y <= BorderWidth) {
            return S;
        } else {
            return CENTER;
        }
    }
}

MouseHoverType Page::CurrentHoverType(glm::ivec2 mouse_pos) {
    if (CurrentSelection == Pieces.end()) {
        return NONE;
    }
    return HoverType(mouse_pos, **CurrentSelection);
}

void Page::HandleLeftClickPress(glm::ivec2 mouse_pos) {
    // Piece that is currently being placed
    if (Placing) {
        Placing = false;
        // Send piece over the wire
        GameObject &piece = **CurrentSelection;
        if (ClientServer::Started()) {
            ClientServer &ns = ClientServer::GetInstance();
            ns.RegisterPageChange("ADD_PIECE", Uid, piece);
        }
        CurrentSelection = Pieces.end();
        return;
    }
    CurrentSelection = Pieces.end();
    for (auto it = Pieces.begin(); it != Pieces.end(); it++) {
        MouseHoverType hover = HoverType(mouse_pos, **it);
        if (hover != NONE && (*it)->Clickable) {
            CurrentSelection = it;
            GameObject &piece = **it;
            if (hover == CENTER) {
                piece.FollowMouse = true;
                piece.ScaleMouse = false;
            } else {
                piece.FollowMouse = false;
                piece.ScaleMouse = true;
                if (hover == N || hover == NE || hover == NW) {
                    (*it)->ScaleEdges.second = -1;
                }
                if (hover == E || hover == NE || hover == SE) {
                    (*it)->ScaleEdges.first = 1;
                }
                if (hover == S || hover == SE || hover == SW) {
                    (*it)->ScaleEdges.second = 1;
                }
                if (hover == W || hover == SW || hover == NW) {
                    (*it)->ScaleEdges.first = -1;
                }
            }

            (*it)->initialSize = (*it)->Size;
            (*it)->initialPos = (*it)->Position;
            DragOrigin = ScreenPosToWorldPos(mouse_pos);
            break;
        }
    }
}

void Page::MoveCurrentSelection(glm::vec2 mouse_pos) {
    static Game &game = Game::GetInstance();
    int inc = game.snapping
              ? 1
              : 8;
    float closest;
    glm::vec2 world_mouse = ScreenPosToWorldPos(mouse_pos);
    if (CurrentSelection != Pieces.end()) {
        GameObject &piece = **CurrentSelection;
        glm::vec2 prev_pos = piece.Position;
        glm::vec2 prev_size = piece.Size;
        if (Placing) {
            piece.Position = world_mouse - piece.Size / 2.0f;
            SnapPieceToGrid(piece, inc);
        } else if (piece.FollowMouse) {
            piece.Position =
                    piece.initialPos + world_mouse - (glm::vec2)DragOrigin;
            SnapPieceToGrid(piece, inc);
        } else if (piece.ScaleMouse) {
            glm::vec2 diff = world_mouse - (glm::vec2)DragOrigin;
            switch (piece.ScaleEdges.first) {
                case 1:
                    piece.Size.x = fmax(
                            piece.initialSize.x + diff.x,
                            TILE_DIMENSIONS / (float)inc);
                    closest = floor(
                            piece.Size.x / (TILE_DIMENSIONS / (float)inc) +
                                    0.5);
                    piece.Size.x = closest * TILE_DIMENSIONS / (float)inc - 2;
                    break;
                case -1:
                    piece.Size.x = fmax(
                            DragOrigin.x + piece.initialSize.x - world_mouse.x,
                            TILE_DIMENSIONS / (float)inc);
                    closest = floor(
                            piece.Size.x / (TILE_DIMENSIONS / (float)inc) +
                                    0.5);
                    piece.Size.x = closest * TILE_DIMENSIONS / (float)inc - 2;
                    piece.Position.x =
                            piece.initialPos.x + piece.initialSize.x -
                                    piece.Size.x;
                    break;
            }
            switch (piece.ScaleEdges.second) {
                case 1:
                    piece.Size.y = fmax(
                            piece.initialSize.y + diff.y,
                            TILE_DIMENSIONS / (float)inc);
                    closest = floor(
                            piece.Size.y / (TILE_DIMENSIONS / (float)inc) +
                                    0.5);
                    piece.Size.y = closest * TILE_DIMENSIONS / (float)inc - 2;
                    break;
                case -1:
                    piece.Size.y = fmax(
                            DragOrigin.y + piece.initialSize.y - world_mouse.y,
                            TILE_DIMENSIONS / (float)inc);
                    closest = floor(
                            piece.Size.y / (TILE_DIMENSIONS / (float)inc) +
                                    0.5);
                    piece.Size.y = closest * TILE_DIMENSIONS / (float)inc - 2;
                    piece.Position.y =
                            piece.initialPos.y + piece.initialSize.y -
                                    piece.Size.y;
                    break;
            }
        }
        else {
            //Arrow key movement
            piece.Position.x += mouse_pos.x;
            piece.Position.y += mouse_pos.y;
        }
        if (ClientServer::Started() && !Placing) {
            static ClientServer &cs = ClientServer::GetInstance();
            if (piece.Position != prev_pos) {
                cs.RegisterPageChange(
                        "MOVE_PIECE",
                        Uid,
                        Util::NetworkData(piece.Position, piece.Uid));
            }
            if (piece.Size != prev_size) {
                cs.RegisterPageChange(
                        "RESIZE_PIECE",
                        Uid,
                        Util::NetworkData(piece.Size, piece.Uid));
            }
        }
    }
}

void Page::HandleLeftClickHold(glm::ivec2 mouse_pos) {
    MoveCurrentSelection(mouse_pos);
}

void Page::HandleLeftClickRelease(glm::ivec2 mouse_pos) {
    (void)mouse_pos;
    if (CurrentSelection != Pieces.end()) {
        (*CurrentSelection)->FollowMouse = false;
        (*CurrentSelection)->ScaleMouse = false;
    }
}

void Page::HandleRightClick(glm::ivec2 mouse_pos) {
    MouseHoverType hover = CurrentHoverType(mouse_pos);
    if (hover != NONE) {
        UserInterface->ClickMenuActive = true;
    } else {
        CurrentSelection = Pieces.end();
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

void Page::HandleArrows(ArrowkeyType key) {
    //ADH - FUTURE come back to this after a decision is made about snapping
    glm::vec2 cardinal(0, 0);
    switch (key) {
    case ArrowkeyType::RIGHT:
        cardinal.x += TILE_DIMENSIONS;
        break;
    case ArrowkeyType::LEFT:
        cardinal.x -= TILE_DIMENSIONS;
        break;
    case ArrowkeyType::DOWN:
        cardinal.y += TILE_DIMENSIONS;
        break;
    case ArrowkeyType::UP:
        cardinal.y -= TILE_DIMENSIONS;
        break;
    }
    MoveCurrentSelection(cardinal);
}

void Page::SnapPieceToGrid(GameObject &piece, int increments) {
    float closest_x = (float)floor(
            piece.Position.x / (TILE_DIMENSIONS / (float)increments) + 0.5);
    float closest_y = (float)floor(
            piece.Position.y / (TILE_DIMENSIONS / (float)increments) + 0.5);
    piece.Position = glm::vec2(
            closest_x * TILE_DIMENSIONS / (float)increments + 1,
            closest_y * TILE_DIMENSIONS / (float)increments + 1);
}

glm::vec2 Page::ScreenPosToWorldPos(glm::ivec2 pos) {
    glm::vec4 world_pos = glm::inverse(View) * glm::vec4(pos, 0.0f, 1.0f);
    return glm::vec2(world_pos.x, world_pos.y);
}

glm::vec2 Page::WorldPosToScreenPos(glm::ivec2 pos) {
    glm::vec4 screen_pos = View * glm::vec4(pos, 0.0f, 1.0f);
    return glm::vec2(screen_pos.x, screen_pos.y);
}

bool Page::Deselect() {
    if (CurrentSelection != Pieces.end()) {
        CurrentSelection = Pieces.end();
        return true;
    }
    return false;
}

