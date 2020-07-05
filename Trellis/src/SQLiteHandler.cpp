#include "sqlite_handler.h"

#include <stdexcept>
#include <utility>

using namespace SQLite;

using std::string, std::runtime_error, std::exchange, std::move, std::swap, std::to_string,
    std::stoll, std::make_shared, std::shared_ptr;

Database::Database(const string &filename)
    : db(make_shared<DB>(filename)) {}

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
    int   result = sqlite3_exec(db->db, SQL.c_str(), callback, udp, &errmsg);
    if (result) {
        err = errmsg;
        sqlite3_free(errmsg);
    }
    return result;
}
Database::Statement
Database::Prepare(std::string SQL) const {
    return Database::Statement(db, std::move(SQL));
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
Database::DB::DB(const string &filename) {
    if (sqlite3_open(filename.c_str(), &db)) {
        throw runtime_error(filename + ": " + sqlite3_errmsg(db));
    }
}
Database::DB::~DB() {
    sqlite3_close(db);
}

Database::Statement::Statement(shared_ptr<DB> db, string sql)
    : db(move(db))
    , SQL(move(sql)) {
    int result = sqlite3_prepare_v2(this->db->db, SQL.c_str(), SQL.size(), &stmt, nullptr);
    if (result) {
        const char *err = sqlite3_errstr(result);
        throw runtime_error(sqlite3_errstr(result));
    }
}

Database::Statement::~Statement() {
    sqlite3_finalize(stmt);
}

void
Database::Statement::Bind(int index) const {
    int result = sqlite3_bind_null(stmt, index);
    if (result) { throw runtime_error(sqlite3_errstr(result)); }
}

void
Database::Statement::Bind(int index, uint64_t value) const {
    sqlite3_int64 signed_val = *reinterpret_cast<const sqlite3_int64 *>(&value);
    int           result     = sqlite3_bind_int64(stmt, index, signed_val);
    if (result) { throw runtime_error(sqlite3_errstr(result)); }
}

void
Database::Statement::Bind(int index, int value) const {
    int result = sqlite3_bind_int(stmt, index, value);
    if (result) { throw runtime_error(sqlite3_errstr(result)); }
}

void
Database::Statement::Bind(int index, unsigned int value) const {
    int result = sqlite3_bind_int64(stmt, index, value);
    if (result) { throw runtime_error(sqlite3_errstr(result)); }
}

void
Database::Statement::Bind(int index, double value) const {
    int result = sqlite3_bind_double(stmt, index, value);
    if (result) { throw runtime_error(sqlite3_errstr(result)); }
}

void
Database::Statement::Bind(int index, const string &value) const {
    int result = sqlite3_bind_text(stmt, index, value.c_str(), value.size(), SQLITE_TRANSIENT);
    if (result) { throw runtime_error(sqlite3_errstr(result)); }
}

void
Database::Statement::Bind(int index, const void *value, size_t size) const {
    // SQLITE_TRANSIENT forces SQLite to make its own internal copy of the data buffer
    int result = sqlite3_bind_blob(stmt, index, value, size, SQLITE_TRANSIENT);
    if (result) { throw runtime_error(sqlite3_errstr(result)); }
}

bool
Database::Statement::Step() {
    if (!done) {
        int result = sqlite3_step(stmt);
        if (result == SQLITE_DONE) {
            done = true;
            row  = false;
        } else if (result != SQLITE_ROW) {
            done = true;
            throw runtime_error(sqlite3_errstr(result));
        } else {
            row = true;
        }
    }
    return done;
}

int
Database::Statement::ColumnSize(int index) {
    if (!row) { throw runtime_error("SQLite3 Statement does not have an available row"); }
    return sqlite3_column_bytes(stmt, index);
}

int
Database::Statement::Column(int index, uint64_t &value) {
    if (!row) { throw runtime_error("SQLite3 Statement does not have an available row"); }
    int     num     = sqlite3_column_count(stmt);
    int64_t sgn_val = sqlite3_column_int64(stmt, index);
    value           = *reinterpret_cast<uint64_t *>(&sgn_val);
    return 0;
}

int
Database::Statement::Column(int index, int &value) {
    if (!row) { throw runtime_error("SQLite3 Statement does not have an available row"); }
    value = sqlite3_column_int(stmt, index);
    return 0;
}

int
Database::Statement::Column(int index, unsigned int &value) {
    if (!row) { throw runtime_error("SQLite3 Statement does not have an available row"); }
    value = sqlite3_column_int64(stmt, index);
    return 0;
}

int
Database::Statement::Column(int index, double &value) {
    if (!row) { throw runtime_error("SQLite3 Statement does not have an available row"); }
    value = sqlite3_column_double(stmt, index);
    return 0;
}

int
Database::Statement::Column(int index, std::string &value) {
    if (!row) { throw runtime_error("SQLite3 Statement does not have an available row"); }
    const unsigned char *txt = sqlite3_column_text(stmt, index);
    int                  len = ColumnSize(index);

    value = string(txt, txt + len);
    return 0;
}

int
Database::Statement::Column(int index, const void *&value) {
    if (!row) { throw runtime_error("SQLite3 Statement does not have an available row"); }
    value = sqlite3_column_blob(stmt, index);
    return 0;
}