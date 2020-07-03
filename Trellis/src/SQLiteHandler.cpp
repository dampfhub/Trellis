#include "sqlite_handler.h"

#include <stdexcept>

using namespace SQLite;

using std::string, std::runtime_error, std::exchange, std::move, std::swap, std::to_string,
    std::stoll;

Database::Database(const string &filename) {
    if (sqlite3_open(filename.c_str(), &db)) {
        throw runtime_error(filename + ": " + sqlite3_errmsg(db));
    }
}

Database::~Database() {
    sqlite3_close(db);
}

Database::Database(Database &&other) noexcept
    : db(exchange(other.db, nullptr)) {}

Database &
Database::operator=(Database &&other) noexcept {
    Database copy(move(other));
    swap(*this, copy);
    return *this;
}

int
Database::Exec(const string &SQL, std::string &err, Database::callback_t callback, void *udp)
    const {
    char *errmsg;
    int   result = sqlite3_exec(db, SQL.c_str(), callback, udp, &errmsg);
    if (result) {
        err = errmsg;
        sqlite3_free(errmsg);
    }
    return result;
}

string
SQLite::from_uint64_t(uint64_t val) {
    int64_t signed_val = *reinterpret_cast<const int64_t *>(&val);
    return to_string(signed_val);
}

uint64_t
SQLite::to_uint64_t(string str) {
    int64_t signed_val = stoll(str);
    return *reinterpret_cast<const uint64_t *>(&signed_val);
}
