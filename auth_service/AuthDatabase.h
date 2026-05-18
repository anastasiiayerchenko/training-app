#ifndef AUTH_DATABASE_H
#define AUTH_DATABASE_H

#include "db/sqlite3.h"
#include <string>
#include <stdexcept>
#include <iostream>

using namespace std;

struct UserData {
    int id;
    string first_name;
    string last_name;
    string username;
    string dob;
};

class IAuthDatabase {
public:
    virtual int registerUser(string first_name, string last_name, string username, string dob, string password) = 0;
    virtual int authenticate(string username, string password) = 0;
    virtual bool isUsernameTaken(string username) = 0;
    virtual UserData getUser(int id) = 0;
    virtual bool changePassword(int id, string old_password, string new_password) = 0;
    virtual bool deleteAccount(int id) = 0;
    virtual ~IAuthDatabase() {}
};

class SqliteAuthDatabase : public IAuthDatabase {
private:
    sqlite3* db;

public:
    SqliteAuthDatabase(string path) { 
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
            cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        }
        string sql = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, first_name TEXT, last_name TEXT, username TEXT UNIQUE, dob TEXT, password TEXT);";
        char* errMsg = 0;
        if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
    }
    ~SqliteAuthDatabase() { sqlite3_close(db); }

    int registerUser(string first_name, string last_name, string username, string dob, string password) override {
        string sql = "INSERT INTO users (first_name, last_name, username, dob, password) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            throw runtime_error(string("Failed to prepare statement: ") + sqlite3_errmsg(db));
        }

        sqlite3_bind_text(stmt, 1, first_name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, last_name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, dob.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, password.c_str(), -1, SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw runtime_error(string("Execution failed: ") + sqlite3_errmsg(db));
        }

        sqlite3_finalize(stmt);
        return (int)sqlite3_last_insert_rowid(db);
    }

    int authenticate(string username, string password) override {
        sqlite3_stmt* stmt;
        string sql = "SELECT id FROM users WHERE username = ? AND password = ?;";
        int id = -1;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                id = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);
        return id;
    }

    bool isUsernameTaken(string username) override {
        sqlite3_stmt* stmt;
        string sql = "SELECT id FROM users WHERE username = ?;";
        bool taken = false;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                taken = true;
            }
        }
        sqlite3_finalize(stmt);
        return taken;
    }

    UserData getUser(int id) override {
        sqlite3_stmt* stmt;
        string sql = "SELECT id, first_name, last_name, username, dob FROM users WHERE id = ?;";
        UserData u;
        u.id = -1;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                u.id = sqlite3_column_int(stmt, 0);
                u.first_name = (const char*)sqlite3_column_text(stmt, 1);
                u.last_name = (const char*)sqlite3_column_text(stmt, 2);
                u.username = (const char*)sqlite3_column_text(stmt, 3);
                u.dob = (const char*)sqlite3_column_text(stmt, 4);
            }
        }
        sqlite3_finalize(stmt);
        return u;
    }

    bool changePassword(int id, string old_password, string new_password) override {
        sqlite3_stmt* stmt;
        string checkSql = "SELECT id FROM users WHERE id = ? AND password = ?;";
        bool valid = false;
        if (sqlite3_prepare_v2(db, checkSql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id);
            sqlite3_bind_text(stmt, 2, old_password.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                valid = true;
            }
        }
        sqlite3_finalize(stmt);

        if (!valid) return false;

        string updateSql = "UPDATE users SET password = ? WHERE id = ?;";
        if (sqlite3_prepare_v2(db, updateSql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, new_password.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, id);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
        return sqlite3_changes(db) > 0;
    }

    bool deleteAccount(int id) override {
        sqlite3_stmt* stmt;
        string sql = "DELETE FROM users WHERE id = ?;";
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
        return sqlite3_changes(db) > 0;
    }
};

#endif
