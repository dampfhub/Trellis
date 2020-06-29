#include "page.h"

#include "board_renderer.h"
#include "client_server.h"
#include "data.h"
#include "Board.h"
#include "resource_manager.h"
#include "sprite_renderer.h"

using std::make_unique, std::move, std::string, std::exchange, std::unique_ptr, std::make_pair,
    std::ref, std::find_if, std::vector, std::byte;

using Data::NetworkData;

Page::Page(string name, glm::vec2 pos, glm::vec2 size, glm::ivec2 cell_dims, uint64_t uid)
    : Name(name)
    , Uid(uid)
    , board_transform(pos, size, 0)
    , cell_dims(cell_dims)
    , board_renderer(this->board_transform, this->View, this->cell_dims) {
    Camera        = make_unique<Camera2D>(200.0f, glm::vec2(0.4f, 2.5f));
    UserInterface = make_unique<PageUI>();
    if (Uid == 0) { Uid = Util::generate_uid(); }
}

Page::~Page() {}

Page::Page(Page &&other) noexcept
    : Name(move(other.Name))
    , board_transform(other.board_transform)
    , Camera(exchange(other.Camera, nullptr))
    , UserInterface(exchange(other.UserInterface, nullptr))
    , Uid(exchange(other.Uid, -1))
    , cell_dims(other.cell_dims)
    , board_renderer(this->board_transform, this->View, this->cell_dims) {}

void
Page::AddPiece(unique_ptr<GameObject> &&piece) {
    GameObject &g   = *piece;
    piece->renderer = make_unique<SpriteRenderer>(piece->transform, this->View, g.Sprite);
    PiecesMap.insert(make_pair(piece->Uid, ref(*piece)));
    Pieces.push_front(move(piece));
}

void
Page::BeginPlacePiece(const Transform &transform, Texture2D sprite) {
    auto piece  = make_unique<GameObject>(transform, sprite);
    mouse_hold  = MouseHoldType::PLACING;
    initialSize = piece->transform.scale;
    initialPos  = piece->transform.position;
    AddPiece(move(piece));
    CurrentSelection = Pieces.begin();
}

void
Page::Update(glm::ivec2 mouse_pos) {
    if (mouse_hold == MouseHoldType::PLACING) { MoveCurrentSelection(mouse_pos); }
    HandleUIEvents();
}

void
Page::Draw() {
    board_renderer.Draw();
    // Draw sprites back-to-front, so the "top" sprite is drawn above the others
    for (auto it = Pieces.rbegin(); it != Pieces.rend(); it++) {
        int border_pixel_width =
            CurrentSelection != Pieces.end() && (*it) == (*CurrentSelection) ? BorderWidth : 0;
        (*it)->Draw(border_pixel_width);
    }
    // Draw user interface
    UserInterface->DrawPieceClickMenu();
}

void
Page::HandleUIEvents() {
    if (UserInterface->MoveToFront) {
        Pieces.splice(Pieces.begin(), Pieces, CurrentSelection);
    } else if (UserInterface->MoveToBack) {
        Pieces.splice(Pieces.end(), Pieces, CurrentSelection);
    }
    UserInterface->ClearFlags();
}

Page::MouseHoverType
Page::HoverType(glm::ivec2 mouse_pos, GameObject &object) {
    glm::ivec2 NW_corner_screen = WorldPosToScreenPos(object.transform.position);
    glm::ivec2 SE_corner_screen =
        WorldPosToScreenPos(object.transform.position + object.transform.scale);
    if (mouse_pos.x < NW_corner_screen.x || mouse_pos.y < NW_corner_screen.y ||
        mouse_pos.x > SE_corner_screen.x || mouse_pos.y > SE_corner_screen.y) {
        return MouseHoverType::NONE;
    }
    glm::ivec2 size_screen = SE_corner_screen - NW_corner_screen;
    glm::ivec2 click_pos   = mouse_pos - (glm::ivec2)NW_corner_screen;
    if (click_pos.x <= BorderWidth) {
        if (click_pos.y <= BorderWidth) {
            return MouseHoverType::NW;
        } else if (size_screen.y - click_pos.y <= BorderWidth) {
            return MouseHoverType::SW;
        } else {
            return MouseHoverType::W;
        }
    } else if (size_screen.x - click_pos.x <= BorderWidth) {
        if (click_pos.y <= BorderWidth) {
            return MouseHoverType::NE;
        } else if (size_screen.y - click_pos.y <= BorderWidth) {
            return MouseHoverType::SE;
        } else {
            return MouseHoverType::E;
        }
    } else {
        if (click_pos.y <= BorderWidth) {
            return MouseHoverType::N;
        } else if (size_screen.y - click_pos.y <= BorderWidth) {
            return MouseHoverType::S;
        } else {
            return MouseHoverType::CENTER;
        }
    }
}

Page::MouseHoverType
Page::CurrentHoverType(glm::ivec2 mouse_pos) {
    if (CurrentSelection == Pieces.end()) { return MouseHoverType::NONE; }
    return HoverType(mouse_pos, **CurrentSelection);
}

void
Page::HandleLeftClickPress(glm::ivec2 mouse_pos) {
    // Piece that is currently being placed
    if (mouse_hold == MouseHoldType::PLACING) {
        mouse_hold = MouseHoldType::NONE;
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
        if (hover != MouseHoverType::NONE && (*it)->Clickable) {
            CurrentSelection  = it;
            GameObject &piece = **it;
            ScaleEdges        = {0, 0};
            if (hover == MouseHoverType::CENTER) {
                mouse_hold = MouseHoldType::FOLLOWING;
            } else {
                mouse_hold = MouseHoldType::SCALING;
                if (hover == MouseHoverType::N || hover == MouseHoverType::NE ||
                    hover == MouseHoverType::NW) {
                    ScaleEdges.second = -1;
                }
                if (hover == MouseHoverType::E || hover == MouseHoverType::NE ||
                    hover == MouseHoverType::SE) {
                    ScaleEdges.first = 1;
                }
                if (hover == MouseHoverType::S || hover == MouseHoverType::SE ||
                    hover == MouseHoverType::SW) {
                    ScaleEdges.second = 1;
                }
                if (hover == MouseHoverType::W || hover == MouseHoverType::SW ||
                    hover == MouseHoverType::NW) {
                    ScaleEdges.first = -1;
                }
            }

            initialSize = (*it)->transform.scale;
            initialPos  = (*it)->transform.position;
            DragOrigin  = ScreenPosToWorldPos(mouse_pos);
            break;
        }
    }
}

void
Page::MoveCurrentSelection(glm::vec2 mouse_pos) {
    int          inc  = Snapping ? 1 : 8;
    float        closest;
    glm::vec2    world_mouse = ScreenPosToWorldPos(mouse_pos);
    if (CurrentSelection != Pieces.end()) {
        GameObject &piece     = **CurrentSelection;
        glm::vec2   prev_pos  = piece.transform.position;
        glm::vec2   prev_size = piece.transform.scale;
        glm::vec2   diff      = world_mouse - (glm::vec2)DragOrigin;
        switch (mouse_hold) {
            case MouseHoldType::PLACING:
                piece.transform.position = world_mouse - piece.transform.scale / 2.0f;
                SnapPieceToGrid(piece, inc);
                break;
            case MouseHoldType::FOLLOWING:
                piece.transform.position = initialPos + world_mouse - (glm::vec2)DragOrigin;
                SnapPieceToGrid(piece, inc);
                break;
            case MouseHoldType::SCALING:
                switch (ScaleEdges.first) {
                    case 1:
                        piece.transform.scale.x =
                            fmax(initialSize.x + diff.x, TILE_DIMENSIONS / (float)inc);
                        closest =
                            floor(piece.transform.scale.x / (TILE_DIMENSIONS / (float)inc) + 0.5);
                        piece.transform.scale.x = closest * TILE_DIMENSIONS / (float)inc - 2;
                        break;
                    case -1:
                        piece.transform.scale.x = fmax(
                            DragOrigin.x + initialSize.x - world_mouse.x,
                            TILE_DIMENSIONS / (float)inc);
                        closest =
                            floor(piece.transform.scale.x / (TILE_DIMENSIONS / (float)inc) + 0.5);
                        piece.transform.scale.x = closest * TILE_DIMENSIONS / (float)inc - 2;
                        piece.transform.position.x =
                            initialPos.x + initialSize.x - piece.transform.scale.x;
                        break;
                }
                switch (ScaleEdges.second) {
                    case 1:
                        piece.transform.scale.y =
                            fmax(initialSize.y + diff.y, TILE_DIMENSIONS / (float)inc);
                        closest =
                            floor(piece.transform.scale.y / (TILE_DIMENSIONS / (float)inc) + 0.5);
                        piece.transform.scale.y = closest * TILE_DIMENSIONS / (float)inc - 2;
                        break;
                    case -1:
                        piece.transform.scale.y = fmax(
                            DragOrigin.y + initialSize.y - world_mouse.y,
                            TILE_DIMENSIONS / (float)inc);
                        closest =
                            floor(piece.transform.scale.y / (TILE_DIMENSIONS / (float)inc) + 0.5);
                        piece.transform.scale.y = closest * TILE_DIMENSIONS / (float)inc - 2;
                        piece.transform.position.y =
                            initialPos.y + initialSize.y - piece.transform.scale.y;
                        break;
                }
                break;
            default:
                piece.transform.position.x += mouse_pos.x;
                piece.transform.position.y += mouse_pos.y;
                break;
        }
        if (ClientServer::Started() && mouse_hold != MouseHoldType::PLACING) {
            static ClientServer &cs = ClientServer::GetInstance();
            if (piece.transform.position != prev_pos) {
                cs.RegisterPageChange(
                    "MOVE_PIECE",
                    Uid,
                    NetworkData(piece.transform.position, piece.Uid));
            }
            if (piece.transform.scale != prev_size) {
                cs.RegisterPageChange(
                    "RESIZE_PIECE",
                    Uid,
                    NetworkData(piece.transform.scale, piece.Uid));
            }
        }
    }
}

void
Page::HandleLeftClickHold(glm::ivec2 mouse_pos) {
    MoveCurrentSelection(mouse_pos);
}

void
Page::HandleLeftClickRelease(glm::ivec2 mouse_pos) {
    (void)mouse_pos;
    if (CurrentSelection != Pieces.end() && mouse_hold != MouseHoldType::PLACING) {
        mouse_hold = MouseHoldType::NONE;
    }
}

void
Page::HandleRightClick(glm::ivec2 mouse_pos) {
    MouseHoverType hover = CurrentHoverType(mouse_pos);
    if (hover != MouseHoverType::NONE) {
        UserInterface->ClickMenuActive = true;
    } else {
        CurrentSelection = Pieces.end();
    }
}

void
Page::HandleMiddleClickPress(glm::ivec2 mouse_pos) {
    DragOrigin = mouse_pos;
}

void
Page::HandleMiddleClickHold(glm::ivec2 mouse_pos) {
    glm::vec2 v = mouse_pos - DragOrigin;
    Camera->Move(v);
    View       = Camera->CalculateView(board_transform.scale);
    DragOrigin = mouse_pos;
}

void
Page::HandleScrollWheel(glm::ivec2 mouse_pos, int scroll_direction) {
    Camera->Zoom(mouse_pos, scroll_direction);
    View = Camera->CalculateView(board_transform.scale);
}

void
Page::HandleArrows(ArrowkeyType key) {
    // ADH - FUTURE come back to this after a decision is made about snapping
    glm::vec2 cardinal(0, 0);
    switch (key) {
        case ArrowkeyType::RIGHT: cardinal.x += TILE_DIMENSIONS; break;
        case ArrowkeyType::LEFT: cardinal.x -= TILE_DIMENSIONS; break;
        case ArrowkeyType::DOWN: cardinal.y += TILE_DIMENSIONS; break;
        case ArrowkeyType::UP: cardinal.y -= TILE_DIMENSIONS; break;
    }
    MoveCurrentSelection(cardinal);
}

void
Page::SnapPieceToGrid(GameObject &piece, int increments) {
    float closest_x =
        (float)floor(piece.transform.position.x / (TILE_DIMENSIONS / (float)increments) + 0.5);
    float closest_y =
        (float)floor(piece.transform.position.y / (TILE_DIMENSIONS / (float)increments) + 0.5);
    piece.transform.position = glm::vec2(
        closest_x * TILE_DIMENSIONS / (float)increments + 1,
        closest_y * TILE_DIMENSIONS / (float)increments + 1);
}

glm::vec2
Page::ScreenPosToWorldPos(glm::ivec2 pos) {
    glm::vec4 world_pos = glm::inverse(View) * glm::vec4(pos, 0.0f, 1.0f);
    return glm::vec2(world_pos.x, world_pos.y);
}

glm::vec2
Page::WorldPosToScreenPos(glm::ivec2 pos) {
    glm::vec4 screen_pos = View * glm::vec4(pos, 0.0f, 1.0f);
    return glm::vec2(screen_pos.x, screen_pos.y);
}

void
Page::SendAllPieces(uint64_t target_uid) {
    if (ClientServer::Started()) {
        std::cout << Pieces.size() << std::endl;
        for (auto piece = Pieces.rbegin(); piece != Pieces.rend(); ++piece) {
            ClientServer &cs = ClientServer::GetInstance();
            cs.RegisterPageChange("ADD_PIECE", Uid, **piece, target_uid);
        }
    }
}

bool
Page::Deselect() {
    if (CurrentSelection != Pieces.end()) {
        if (mouse_hold == MouseHoldType::PLACING) {
            PiecesMap.erase((*CurrentSelection)->Uid);
            Pieces.erase(CurrentSelection);
            mouse_hold = MouseHoldType::NONE;
        }
        CurrentSelection = Pieces.end();
        return true;
    }
    return false;
}

void
Page::DeletePiece(uint64_t uid) {
    auto piece_it = find_if(Pieces.begin(), Pieces.end(), [uid](unique_ptr<GameObject> &g) {
        return g->Uid == uid;
    });
    if (piece_it != Pieces.end()) { Pieces.erase(piece_it); }
    PiecesMap.erase(uid);
    CurrentSelection = Pieces.end();
}

void
Page::DeleteCurrentSelection() {
    if (CurrentSelection != Pieces.end()) {
        if (mouse_hold != MouseHoldType::PLACING) {
            static ClientServer &cs = ClientServer::GetInstance();
            cs.RegisterPageChange(
                "DELETE_PIECE",
                Uid,
                NetworkData(*CurrentSelection, (*CurrentSelection)->Uid));
            DeletePiece((*CurrentSelection)->Uid);
        } else {
            Deselect();
        }
    }
}

vector<byte>
Page::Serialize() const {
    vector<vector<byte>> bytes;
    bytes.push_back(Util::serialize_vec(Uid));
    bytes.push_back(Util::serialize_vec(board_transform));
    bytes.push_back(Util::serialize_vec(cell_dims));
    bytes.push_back(Util::serialize_vec(Name));
    return Util::flatten(bytes);
}

Page
Page::deserialize_impl(const vector<byte> &vec) {
    auto ptr       = vec.data();
    auto end       = ptr + vec.size();
    auto uid       = Util::deserialize<uint64_t>(ptr);
    auto t         = Util::deserialize<Transform>(ptr += sizeof(uid));
    auto cell_dims = Util::deserialize<glm::ivec2>(ptr += sizeof(t));
    auto name      = Util::deserialize<string>(vector(ptr += sizeof(cell_dims), end));
    return Page(name, t.position, t.scale, cell_dims, uid);
}

glm::ivec2
Page::getCellDims() const {
    return cell_dims;
}

void
Page::setCellDims(glm::ivec2 cellDims) {
    if (cellDims.x < 1) cellDims.x = 1;
    if (cellDims.y < 1) cellDims.y = 1;
    cell_dims             = cellDims;
    board_transform.scale = glm::vec2(cell_dims) * TILE_DIMENSIONS;
}
void
Page::CopySettingsFromPage(const Page &other) {
    board_transform = other.board_transform;
    cell_dims = other.cell_dims;
    Name = other.Name;
    board_renderer.CellDims = cell_dims;
    board_renderer.setTransform(board_transform);
}
