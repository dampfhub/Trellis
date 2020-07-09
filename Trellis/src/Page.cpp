#include <utility>

#include "page.h"
#include "client_server.h"
#include "data.h"
#include "resource_manager.h"

using std::make_unique, std::move, std::string, std::exchange, std::unique_ptr, std::make_pair,
    std::ref, std::find_if, std::vector, std::byte, std::to_string, std::stof, std::stoi,
    std::shared_ptr;

using Data::NetworkData;

Page::Page(const CorePage &other)
    : CorePage(other)
    , board_renderer(this->board_transform, this->View, this->cell_dims) {
    Camera        = make_unique<Camera2D>(200.0f, glm::vec2(0.4f, 2.5f));
    UserInterface = make_unique<PageUI>();
    if (Uid == 0) { Uid = Util::generate_uid(); }
}

Page &
Page::operator=(const CorePage &other) {
    (CorePage &)*this     = other;
    board_transform.scale = glm::vec2(cell_dims) * TILE_DIMENSIONS;
    return *this;
}

Page::~Page() {}

GameObject &
Page::AddPiece(const CoreGameObject &core_piece) {
    auto obj = make_unique<GameObject>(core_piece, View);
    PiecesMap.insert(make_pair(obj->Uid, ref(*obj)));
    Pieces.push_front(move(obj));
    return *Pieces.front();
}

void
Page::BeginPlacePiece(const Transform &transform, const shared_ptr<Texture2D> &sprite) {
    auto  core       = CoreGameObject(transform, sprite->ImageUID, 0, true, glm::vec3(1));
    auto &piece      = AddPiece(core);
    mouse_hold       = MouseHoldType::PLACING;
    initialSize      = piece.transform.scale;
    initialPos       = piece.transform.position;
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
            ns.ChannelPublish("ADD_PIECE", Uid, (CoreGameObject)piece);
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
    int       inc = Snapping ? 1 : 8;
    float     closest;
    glm::vec2 world_mouse = ScreenPosToWorldPos(mouse_pos);
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
                            floor(piece.transform.scale.x / (TILE_DIMENSIONS / (float)inc) + 0.5f);
                        piece.transform.scale.x = closest * TILE_DIMENSIONS / (float)inc - 2;
                        break;
                    case -1:
                        piece.transform.scale.x = fmax(
                            DragOrigin.x + initialSize.x - world_mouse.x,
                            TILE_DIMENSIONS / (float)inc);
                        closest =
                            floor(piece.transform.scale.x / (TILE_DIMENSIONS / (float)inc) + 0.5f);
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
                            floor(piece.transform.scale.y / (TILE_DIMENSIONS / (float)inc) + 0.5f);
                        piece.transform.scale.y = closest * TILE_DIMENSIONS / (float)inc - 2;
                        break;
                    case -1:
                        piece.transform.scale.y = fmax(
                            DragOrigin.y + initialSize.y - world_mouse.y,
                            TILE_DIMENSIONS / (float)inc);
                        closest =
                            floor(piece.transform.scale.y / (TILE_DIMENSIONS / (float)inc) + 0.5f);
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
                cs.ChannelPublish(
                    "MOVE_PIECE",
                    Uid,
                    NetworkData(piece.transform.position, piece.Uid));
            }
            if (piece.transform.scale != prev_size) {
                cs.ChannelPublish(
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
        for (auto piece = Pieces.rbegin(); piece != Pieces.rend(); ++piece) {
            ClientServer &  cs   = ClientServer::GetInstance();
            CoreGameObject &core = **piece;
            cs.ChannelPublish("ADD_PIECE", Uid, core, target_uid);
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
            cs.ChannelPublish(
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
CorePage::Serialize() const {
    vector<vector<byte>> bytes;
    bytes.push_back(Util::serialize_vec(Uid));
    bytes.push_back(Util::serialize_vec(board_transform));
    bytes.push_back(Util::serialize_vec(cell_dims));
    bytes.push_back(Util::serialize_vec(Name));
    return Util::flatten(bytes);
}

CorePage
CorePage::deserialize_impl(const vector<byte> &vec) {
    auto ptr       = vec.data();
    auto end       = ptr + vec.size();
    auto uid       = Util::deserialize<uint64_t>(ptr);
    auto t         = Util::deserialize<Transform>(ptr += sizeof(uid));
    auto cell_dims = Util::deserialize<glm::ivec2>(ptr += sizeof(t));
    auto name      = Util::deserialize<string>(vector(ptr += sizeof(cell_dims), end));
    return CorePage(name, t, cell_dims, uid);
}
CorePage::CorePage(
    std::string       name,
    const Transform & boardTransform,
    const glm::ivec2 &cellDims,
    uint64_t          uid)
    : Uid(uid)
    , Name(std::move(name))
    , board_transform(boardTransform)
    , cell_dims(cellDims) {}

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
Page::WriteToDB(const SQLite::Database &db, uint64_t game_id) const {
    auto stmt = db.Prepare("INSERT OR REPLACE INTO Pages VALUES(?,?,?,?,?,?,?,?,?,?);");
    stmt.Bind(1, Uid);
    stmt.Bind(2, Name);
    stmt.Bind(3, game_id);
    stmt.Bind(4, board_transform.position.x);
    stmt.Bind(5, board_transform.position.y);
    stmt.Bind(6, board_transform.scale.x);
    stmt.Bind(7, board_transform.scale.y);
    stmt.Bind(8, board_transform.rotation);
    stmt.Bind(9, cell_dims.x);
    stmt.Bind(10, cell_dims.y);
    stmt.Step();

    auto get_pieces = db.Prepare("SELECT id FROM GameObjects WHERE page_id = ?;");
    get_pieces.Bind(1, Uid);

    while (!get_pieces.Step()) {
        uint64_t piece_id;
        get_pieces.Column(0, piece_id);
        if (PiecesMap.find(piece_id) == PiecesMap.end()) {
            auto del_piece = db.Prepare("DELETE FROM GameObjects WHERE id = ?;");
            del_piece.Bind(1, piece_id);
            del_piece.Step();
        }
    }

    for (auto &piece : Pieces) { piece->WriteToDB(db, Uid); }
}

CorePage::CorePage(const SQLite::Database &db, uint64_t page_id)
    : Uid(page_id) {
    using SQLite::from_uint64_t, SQLite::to_uint64_t;

    auto page_callback = [](void *udp, int count, char **values, char **names) -> int {
        auto core = static_cast<CorePage *>(udp);

        assert(!strcmp(names[0], "id"));
        core->Uid = to_uint64_t(values[0]);

        assert(!strcmp(names[1], "name"));
        core->Name = values[1];

        //[2] = "game_id"

        assert(!strcmp(names[3], "t_pos_x"));
        core->board_transform.position.x = stof(values[3]);

        assert(!strcmp(names[4], "t_pos_y"));
        core->board_transform.position.y = stof(values[4]);

        assert(!strcmp(names[5], "t_scale_x"));
        core->board_transform.scale.x = stof(values[5]);

        assert(!strcmp(names[6], "t_scale_y"));
        core->board_transform.scale.y = stof(values[6]);

        assert(!strcmp(names[7], "t_rotation"));
        core->board_transform.rotation = stof(values[7]);

        assert(!strcmp(names[8], "cell_x"));
        core->cell_dims.x = stoi(values[8]);

        assert(!strcmp(names[9], "cell_y"));
        core->cell_dims.y = stoi(values[9]);

        return 0;
    };
    std::string err;
    int         result = db.Exec(
        "SELECT * FROM Pages WHERE id = " + from_uint64_t(page_id),
        err,
        +page_callback,
        this);
    if (result) { std::cerr << err << std::endl; }
    assert(!result);
}

Page::Page(const SQLite::Database &db, uint64_t uid)
    : CorePage(db, uid)
    , board_renderer(this->board_transform, this->View, this->cell_dims) {
    using SQLite::from_uint64_t;
    using SQLite::to_uint64_t;

    Camera        = make_unique<Camera2D>(200.0f, glm::vec2(0.4f, 2.5f));
    UserInterface = make_unique<PageUI>();

    auto piece_callback = [](void *udp, int count, char **values, char **names) -> int {
        auto piece_uids = static_cast<std::list<uint64_t> *>(udp);

        assert(!strcmp(names[0], "id"));
        piece_uids->push_back(to_uint64_t(values[0]));
        return 0;
    };
    std::list<uint64_t> piece_uids;
    std::string         err;
    int                 result = db.Exec(
        "SELECT id FROM GameObjects WHERE page_id = " + from_uint64_t(Uid),
        err,
        +piece_callback,
        &piece_uids);
    if (result) { std::cerr << err << std::endl; }
    assert(!result);
    for (auto piece_uid : piece_uids) { AddPiece(CoreGameObject(db, piece_uid)); }
}
