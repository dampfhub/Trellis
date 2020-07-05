#include "client_server.h"
#include "resource_manager.h"
#include "game_object.h"
#include "sprite_renderer.h"
#include "util.h"

#include <string>

using std::move, std::exchange, std::make_unique, std::vector, std::byte, std::to_string, std::stoi,
    std::stod, std::stof;

CoreGameObject::CoreGameObject(
    const Transform &transform,
    uint64_t         sprite_uid,
    uint64_t         uid,
    bool             clickable,
    glm::vec3        color)
    : transform(transform)
    , Color(color)
    , Uid(uid)
    , Clickable(clickable)
    , SpriteUid(sprite_uid) {}

vector<byte>
CoreGameObject::Serialize() const {
    std::vector<std::vector<byte>> bytes;
    bytes.push_back(Util::serialize_vec(transform));
    bytes.push_back(Util::serialize_vec(Color));
    bytes.push_back(Util::serialize_vec(Uid));
    bytes.push_back(Util::serialize_vec(Clickable));
    bytes.push_back(Util::serialize_vec(SpriteUid));
    return Util::flatten(bytes);
}

CoreGameObject
CoreGameObject::deserialize_impl(const vector<byte> &vec) {
    CoreGameObject g;
    const byte *   ptr = vec.data();
    g.transform        = Util::deserialize<Transform>(ptr);
    g.Color            = Util::deserialize<glm::vec3>(ptr += sizeof(g.transform));
    g.Uid              = Util::deserialize<uint64_t>(ptr += sizeof(g.Color));
    g.Clickable        = Util::deserialize<bool>(ptr += sizeof(g.Uid));
    g.SpriteUid        = Util::deserialize<uint64_t>(ptr += sizeof(g.Clickable));
    return g;
}

GameObject::GameObject(const CoreGameObject &other) {
    static ResourceManager &rm = ResourceManager::GetInstance();
    static ClientServer &   cs = ClientServer::GetInstance();
    (CoreGameObject &)*this    = other;
    if (Uid == 0) Uid = Util::generate_uid();
    if (rm.Images.find(SpriteUid) == rm.Images.end()) {
        // Image isn't cached, need to request it
        cs.RegisterPageChange("IMAGE_REQUEST", cs.uid, SpriteUid);
    } else {
        // Image is cached, just grab it from the resource manager
        Sprite = rm.GetTexture(SpriteUid);
    }
}

GameObject::GameObject(const CoreGameObject &other, glm::mat4 &View)
    : GameObject(other) {
    renderer = make_unique<SpriteRenderer>(transform, View, Sprite);
}

GameObject &
GameObject::operator=(const CoreGameObject &other) {
    (CoreGameObject &)*this = other;
    return *this;
}

void
GameObject::Draw(int border_pixel_width) {
    this->renderer->shader->SetInteger("border_width", border_pixel_width);
    this->renderer->Draw();
}

GameObject::GameObject(GameObject &&other) noexcept
    : Sprite(other.Sprite)
    , renderer(exchange(other.renderer, nullptr)) {
    transform = other.transform;
    Color     = other.Color;
    Uid       = exchange(other.Uid, -1);
    Clickable = other.Clickable;
}

GameObject &
GameObject::operator=(GameObject &&other) noexcept {
    GameObject copy(move(other));
    copy.swap(*this);
    return *this;
}

void
GameObject::swap(GameObject &other) {
    using std::swap;
    swap(transform, other.transform);
    swap(Color, other.Color);
    swap(Uid, other.Uid);
    swap(Sprite, other.Sprite);
    swap(Clickable, other.Clickable);
    swap(renderer, other.renderer);
}
void
GameObject::WriteToDB(const SQLite::Database &db, uint64_t page_id) const {
    auto stmt = db.Prepare("INSERT OR REPLACE INTO GameObjects VALUES(?,?,?,?,?,?,?,?,?,?,?,?);");
    stmt.Bind(1, Uid);
    stmt.Bind(2, Clickable);
    stmt.Bind(3, SpriteUid);
    stmt.Bind(4, page_id);
    stmt.Bind(5, transform.position.x);
    stmt.Bind(6, transform.position.y);
    stmt.Bind(7, transform.scale.x);
    stmt.Bind(8, transform.scale.y);
    stmt.Bind(9, transform.rotation);
    stmt.Bind(10, Color.x);
    stmt.Bind(11, Color.y);
    stmt.Bind(12, Color.z);
    stmt.Step();
}

CoreGameObject::CoreGameObject(const SQLite::Database &db, uint64_t uid) {
    static ResourceManager &rm = ResourceManager::GetInstance();
    using SQLite::from_uint64_t;
    auto callback = [](void *udp, int count, char **values, char **names) -> int {
        using SQLite::to_uint64_t;

        auto core = static_cast<CoreGameObject *>(udp);

        assert(!strcmp(names[0], "id"));
        core->Uid = to_uint64_t(values[0]);

        assert(!strcmp(names[1], "clickable"));
        core->Clickable = stoi(values[1]);

        assert(!strcmp(names[2], "sprite"));
        core->SpriteUid = to_uint64_t(values[2]);

        //[3] = "page_id"

        assert(!strcmp(names[4], "t_pos_x"));
        core->transform.position.x = stod(values[4]);

        assert(!strcmp(names[5], "t_pos_y"));
        core->transform.position.y = stod(values[5]);

        assert(!strcmp(names[6], "t_scale_x"));
        core->transform.scale.x = stod(values[6]);

        assert(!strcmp(names[7], "t_scale_y"));
        core->transform.scale.y = stod(values[7]);

        assert(!strcmp(names[8], "t_rotation"));
        core->transform.rotation = stof(values[8]);

        assert(!strcmp(names[9], "color_x"));
        core->Color.x = stod(values[9]);

        assert(!strcmp(names[10], "color_y"));
        core->Color.y = stod(values[10]);

        assert(!strcmp(names[11], "color_z"));
        core->Color.z = stod(values[11]);
        return 0;
    };
    std::string err;
    int         result =
        db.Exec("SELECT * FROM GameObjects WHERE id = " + from_uint64_t(uid), err, +callback, this);
    if (result) { std::cerr << err << std::endl; }
    rm.ReadFromDB(db, SpriteUid);
}

void
swap(GameObject &a, GameObject &b) {
    a.swap(b);
}
