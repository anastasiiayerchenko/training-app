#ifndef AUTH_DATABASE_H
#define AUTH_DATABASE_H

#include "db/sqlite3.h"
#include <string>

using namespace std;

struct UserData {
    int id;
    string first_name;
    string last_name;
    string username;
    int age;
};

class IAuthDatabase {
public:
    virtual int registerUser(string first_name, string last_name, string username, int age, string password) = 0;
    virtual int authenticate(string username, string password) = 0;
    virtual bool isUsernameTaken(string username) = 0;
    virtual UserData getUser(int id) = 0;
    virtual ~IAuthDatabase() {}
};

class SqliteAuthDatabase : public IAuthDatabase {
private:
    sqlite3* db;

public:
    SqliteAuthDatabase(string path) { sqlite3_open(path.c_str(), &db); }
    ~SqliteAuthDatabase() { sqlite3_close(db); }

    int registerUser(string first_name, string last_name, string username, int age, string password) override {
        string sql = "INSERT INTO users (first_name, last_name, username, age, password) VALUES ('"
            + first_name + "', '" + last_name + "', '" + username + "', " + to_string(age) + ", '" + password + "');";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
        return (int)sqlite3_last_insert_rowid(db);
    }

    int authenticate(string username, string password) override {
        sqlite3_stmt* stmt;
        string sql = "SELECT id FROM users WHERE username = '" + username + "' AND password = '" + password + "';";
        int id = -1;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                id = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);
        return id;
    }

    bool isUsernameTaken(string username) override {
        sqlite3_stmt* stmt;
        string sql = "SELECT id FROM users WHERE username = '" + username + "';";
        bool taken = false;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                taken = true;
            }
        }
        sqlite3_finalize(stmt);
        return taken;
    }

    UserData getUser(int id) override {
        sqlite3_stmt* stmt;
        string sql = "SELECT id, first_name, last_name, username, age FROM users WHERE id = " + to_string(id) + ";";
        UserData u;
        u.id = -1;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                u.id = sqlite3_column_int(stmt, 0);
                u.first_name = (const char*)sqlite3_column_text(stmt, 1);
                u.last_name = (const char*)sqlite3_column_text(stmt, 2);
                u.username = (const char*)sqlite3_column_text(stmt, 3);
                u.age = sqlite3_column_int(stmt, 4);
            }
        }
        sqlite3_finalize(stmt);
        return u;
    }
};

#endif
