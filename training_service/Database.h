#ifndef DATABASE_H
#define DATABASE_H

#include "Exercise.h"
#include "Training.h"
#include "db/sqlite3.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class IDatabase {
public:
  virtual void addTraining(int athlete_id, string type, string muscle,
                           string date, int exp) = 0;
  virtual void fillTraining(Training *tr, string muscle, int exp) = 0;
  virtual vector<Training *> getAthleteTrainings(int athlete_id) = 0;
  virtual ~IDatabase() {}
};

class SqliteDatabase : public IDatabase {
private:
  sqlite3 *db;

public:
  SqliteDatabase(string path) { sqlite3_open(path.c_str(), &db); }
  ~SqliteDatabase() { sqlite3_close(db); }



  void addTraining(int athlete_id, string type, string muscle, string date,
                   int exp) override {
    string sql =
        "INSERT INTO trainings (athlete_id, type, muscle, date, exp) VALUES (" +
        to_string(athlete_id) + ",'" + type + "','" + muscle + "','" + date +
        "'," + to_string(exp) + ");";
    sqlite3_exec(db, sql.c_str(), 0, 0, 0);
  }

  void fillTraining(Training *tr, string muscle, int exp) override {
    sqlite3_stmt *stmt;
    string sql =
        "SELECT name, sets, reps, duration FROM exercises WHERE type='" +
        tr->getType() + "' AND (muscle='" + muscle +
        "' OR muscle='-') AND exp=" + to_string(exp) + ";";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        string n = (const char *)sqlite3_column_text(stmt, 0);
        int sets = sqlite3_column_int(stmt, 1);
        int reps = sqlite3_column_int(stmt, 2);
        const char *dur = (const char *)sqlite3_column_text(stmt, 3);

        string durationValue = (dur != NULL) ? dur : "-";
        tr->addExercise(ExerciseLibrary::createExercise(tr->getType(), n, sets,
                                                        reps, durationValue));
      }
    }
    sqlite3_finalize(stmt);
  }



  vector<Training *> getAthleteTrainings(int athlete_id) override {
    vector<Training *> history;
    sqlite3_stmt *stmt;
    string sql =
        "SELECT type, muscle, date, exp FROM trainings WHERE athlete_id = " +
        to_string(athlete_id) + ";";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        string t = (const char *)sqlite3_column_text(stmt, 0);
        string m = (const char *)sqlite3_column_text(stmt, 1);
        string d = (const char *)sqlite3_column_text(stmt, 2);
        int e = sqlite3_column_int(stmt, 3);

        Training *tr;
        if (t == "Strength")
          tr = new StrengthTraining(d, m);
        else
          tr = new Training(d, t);

        fillTraining(tr, m, e);
        history.push_back(tr);
      }
    }
    sqlite3_finalize(stmt);
    return history;
  }


};

#endif