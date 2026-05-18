#ifndef RECORD_DATABASE_H
#define RECORD_DATABASE_H

#include "record.h"
#include "db/sqlite3.h"
#include <string>
#include <vector>
using namespace std;

class IRecordDatabase {
public:
    virtual int addEntry(int athlete_id, string exercise_name, double weight, string date) = 0;
    virtual vector<Record> getHistory(int athlete_id, string exercise_name) = 0;
    virtual vector<BestRecord> getBestRecords(int athlete_id) = 0;
    virtual bool deleteEntry(int id) = 0;
    virtual void deleteAthleteEntries(int athlete_id) = 0;
    virtual vector<string> getRecordedExercises(int athlete_id) = 0;
    virtual ~IRecordDatabase() {}
};



class SqliteRecordDatabase : public IRecordDatabase {
private:
    sqlite3* db;

public:
    SqliteRecordDatabase(string path) { sqlite3_open(path.c_str(), &db); }
    ~SqliteRecordDatabase() { sqlite3_close(db); }

    int addEntry(int athlete_id, string exercise_name, double weight, string date) override {
        string sql = "INSERT INTO entries (athlete_id, exercise_name, weight, date) VALUES ("
            + to_string(athlete_id) + ",'" + exercise_name + "',"
            + to_string(weight) + ",'" + date + "');";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
        return (int)sqlite3_last_insert_rowid(db);
    }

    vector<Record> getHistory(int athlete_id, string exercise_name) override {
        vector<Record> history;
        sqlite3_stmt* stmt;
        string sql = "SELECT id, athlete_id, exercise_name, weight, date FROM entries WHERE athlete_id = "
            + to_string(athlete_id) + " AND exercise_name = '" + exercise_name
            + "' ORDER BY date;";

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                Record r;
                r.id = sqlite3_column_int(stmt, 0);
                r.athlete_id = sqlite3_column_int(stmt, 1);
                r.exercise_name = (const char*)sqlite3_column_text(stmt, 2);
                r.weight = sqlite3_column_double(stmt, 3);
                r.date = (const char*)sqlite3_column_text(stmt, 4);
                history.push_back(r);
            }
        }
        sqlite3_finalize(stmt);
        return history;
    }

    vector<BestRecord> getBestRecords(int athlete_id) override {
        vector<BestRecord> records;
        sqlite3_stmt* stmt;
        string sql = "SELECT e.exercise_name, e.weight, e.date FROM entries e "
            "INNER JOIN (SELECT exercise_name, MAX(weight) as max_weight FROM entries "
            "WHERE athlete_id = " + to_string(athlete_id) + " GROUP BY exercise_name) m "
            "ON e.exercise_name = m.exercise_name AND e.weight = m.max_weight "
            "AND e.athlete_id = " + to_string(athlete_id) + ";";

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                BestRecord br;
                br.exercise_name = (const char*)sqlite3_column_text(stmt, 0);
                br.best_weight = sqlite3_column_double(stmt, 1);
                br.date = (const char*)sqlite3_column_text(stmt, 2);
                records.push_back(br);
            }
        }
        sqlite3_finalize(stmt);
        return records;
    }

    bool deleteEntry(int id) override {
        string sql = "DELETE FROM entries WHERE id = " + to_string(id) + ";";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
        return sqlite3_changes(db) > 0;
    }

    void deleteAthleteEntries(int athlete_id) override {
        string sql = "DELETE FROM entries WHERE athlete_id = " + to_string(athlete_id) + ";";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    }

    vector<string> getRecordedExercises(int athlete_id) override {
        vector<string> exercises;
        sqlite3_stmt* stmt;
        string sql = "SELECT DISTINCT exercise_name FROM entries WHERE athlete_id = " + to_string(athlete_id) + ";";
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const char* name = (const char*)sqlite3_column_text(stmt, 0);
                if (name) exercises.push_back(name);
            }
        }
        sqlite3_finalize(stmt);
        return exercises;
    }
};

#endif
