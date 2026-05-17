#ifndef RECORD_H
#define RECORD_H

#include <string>
#include "json.hpp"
using json = nlohmann::json;

struct Record {
    int id;
    int athlete_id;
    std::string exercise_name;
    double weight;
    std::string date;

    Record() {
        id = 0;
        athlete_id = 0;
        exercise_name = "";
        weight = 0.0;
        date = "";
    }

    Record(int r_id, int a_id, std::string name, double w, std::string d) {
        id = r_id;
        athlete_id = a_id;
        exercise_name = name;
        weight = w;
        date = d;
    }

    json toJson() const {
        return json{
            {"id", id},
            {"athlete_id", athlete_id},
            {"exercise", exercise_name},
            {"weight", weight},
            {"date", date}
        };
    }
};

struct BestRecord {
    std::string exercise_name;
    double best_weight;
    std::string date;

    json toJson() const {
        return json{
            {"exercise", exercise_name},
            {"best", best_weight},
            {"date", date}
        };
    }
};

#endif