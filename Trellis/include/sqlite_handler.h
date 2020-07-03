#ifndef TRELLIS_SQLITE_HANDLER_H
#define TRELLIS_SQLITE_HANDLER_H

#include <string>
#include <sqlite3.h>

namespace SQLite {

class Database {
public:
    explicit Database(const std::string &filename);
    Database(Database &&other) noexcept;
    Database &operator=(Database &&other) noexcept;
    ~Database();

    Database(const Database &other) = delete;
    Database &operator=(const Database &other) = delete;

private:
    sqlite3 *db;
};

} // namespace SQLite

#endif // TRELLIS_SQLITE_HANDLER_H
