#ifndef TRELLIS_SQLITE_HANDLER_H
#define TRELLIS_SQLITE_HANDLER_H

#include <string>
#include <sqlite3.h>
#include <memory>

namespace SQLite {

class Database {
public:
    typedef int (*callback_t)(void *udp, int count, char **values, char **names);
    explicit Database(const std::string &filename);
    Database(Database &&other) noexcept;
    Database &operator=(Database &&other) noexcept;
    ~Database()       = default;

    Database(const Database &other) = delete;
    Database &operator=(const Database &other) = delete;

    int Exec(
        const std::string &SQL,
        std::string &      err,
        callback_t         callback = nullptr,
        void *             udp      = nullptr) const;

private:
    class DB {
        friend class Database;

    public:
        explicit DB(const std::string &filename);
        ~DB();

        DB(const DB &other) = delete;
        DB &operator=(const DB &other) = delete;

    private:
        sqlite3 *db;
    };
    std::shared_ptr<DB> db;

    class Statement {
    public:
        Statement(std::shared_ptr<DB> db, std::string SQL);
        ~Statement();

        Statement(const Statement &other) = delete;
        Statement &operator=(const Statement &other) = delete;

        // Index starts from 1
        void Bind(int index) const;
        void Bind(int index, uint64_t value) const;
        void Bind(int index, int value) const;
        void Bind(int index, unsigned int value) const;
        void Bind(int index, double value) const;
        void Bind(int index, const std::string &value) const;
        void Bind(int index, const void *value, size_t size) const;

        bool Step();

        // Index starts from 0
        int ColumnSize(int index);
        int Column(int index, uint64_t &value);
        int Column(int index, int &value);
        int Column(int index, unsigned int &value);
        int Column(int index, double &value);
        int Column(int index, std::string &value);
        int Column(int index, const void *&value);

    private:
        std::shared_ptr<DB> db;
        sqlite3_stmt *      stmt;
        std::string         SQL;
        bool                done = false;
        bool                row  = false;
    };

public:
    Statement Prepare(std::string SQL) const;
};

std::string from_uint64_t(uint64_t val);
uint64_t    to_uint64_t(std::string str);

} // namespace SQLite

#endif // TRELLIS_SQLITE_HANDLER_H
