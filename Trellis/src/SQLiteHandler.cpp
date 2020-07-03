#include "sqlite_handler.h"

#include <stdexcept>

using namespace SQLite;

using std::string, std::runtime_error, std::exchange, std::move, std::swap;

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
Database::Exec(const string &SQL, std::string &err, Database::callback_t callback, void *udp) const {
    char *errmsg;
    int   result = sqlite3_exec(db, SQL.c_str(), callback, udp, &errmsg);
    if (result) {
        err = errmsg;
        sqlite3_free(errmsg);
    }
    return result;
}
