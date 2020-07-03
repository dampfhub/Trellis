#ifndef TRELLIS_SQLITE_HANDLER_H
#define TRELLIS_SQLITE_HANDLER_H

#include <string>
#include <sqlite3.h>

namespace SQLite {

class Database {
public:
    typedef int (*callback_t)(void *udp, int count, char **values, char **names);
    explicit Database(const std::string &filename);
    Database(Database &&other) noexcept;
    Database &operator=(Database &&other) noexcept;
    ~Database();

    Database(const Database &other) = delete;
    Database &operator=(const Database &other) = delete;

    int Exec(
        const std::string &SQL,
        std::string &      err,
        callback_t         callback = nullptr,
        void *             udp      = nullptr) const;

private:
    sqlite3 *db;
};

} // namespace SQLite

#endif // TRELLIS_SQLITE_HANDLER_H
