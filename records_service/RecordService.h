#ifndef RECORD_SERVICE_H
#define RECORD_SERVICE_H

#include <string>
#include <vector>
#include <stdexcept>
#include <functional>
#include <unordered_set>
#include <regex>
#include "RecordDatabase.h"
#include "httplib.h"
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

class RecordService {
private:
    IRecordDatabase* db;
    string trainingServiceUrl;

public:
    RecordService(IRecordDatabase* database, string tsUrl = "http://localhost:8080") {
        db = database;
        trainingServiceUrl = tsUrl;
    }

    int addEntry(int athlete_id, string exercise_name, double weight, string date) {
        if (exercise_name.empty()) throw invalid_argument("Exercise name cannot be empty!");
        if (weight <= 0) throw invalid_argument("Weight must be positive!");
        if (date.empty()) throw invalid_argument("Date cannot be empty!");
        
        regex date_pattern(R"(\d{4}-\d{2}-\d{2})");
        if (!regex_match(date, date_pattern)) {
            throw invalid_argument("Date must be in YYYY-MM-DD format!");
        }

        return db->addEntry(athlete_id, exercise_name, weight, date);
    }

    vector<Record> getHistory(int athlete_id, string exercise_name) {
        return db->getHistory(athlete_id, exercise_name);
    }

    vector<BestRecord> getBestRecords(int athlete_id) {
        return db->getBestRecords(athlete_id);
    }

    bool deleteEntry(int id) {
        return db->deleteEntry(id);
    }

    vector<string> getAvailableExercises(int athlete_id) {
        unordered_set<string> unique_exercises;

        // 1. Get exercises from local records DB
        vector<string> recorded = db->getRecordedExercises(athlete_id);
        for (const string& ex : recorded) {
            unique_exercises.insert(ex);
        }

        // 2. Get exercises from Training Service
        httplib::Client cli(trainingServiceUrl);
        auto res = cli.Get("/athletes/" + to_string(athlete_id) + "/trainings");
        
        if (res && res->status == 200) {
            try {
                json responseJson = json::parse(res->body);
                if (responseJson.contains("trainings") && responseJson["trainings"].is_array()) {
                    for (const auto& training : responseJson["trainings"]) {
                        if (training.contains("exercises") && training["exercises"].is_array()) {
                            for (const auto& ex : training["exercises"]) {
                                if (ex.contains("name")) {
                                    unique_exercises.insert(ex["name"].get<string>());
                                }
                            }
                        }
                    }
                }
            } catch (const exception& e) {
                // If parsing fails or something else goes wrong, we still return the local records
            }
        }

        // Convert set back to vector
        vector<string> result(unique_exercises.begin(), unique_exercises.end());
        return result;
    }
};

class IoCContainer {
private:
    vector<pair<string, void*>> entries;
    vector<function<void()>> deleters;

public:
    template<typename Interface>
    void registerInstance(Interface* instance) {
        entries.push_back({typeid(Interface).name(), (void*)instance});
        deleters.push_back([instance]() { delete instance; });
    }

    template<typename T>
    T* resolve() {
        string key = typeid(T).name();
        for (size_t i = 0; i < entries.size(); i++) {
            if (entries[i].first == key) {
                return static_cast<T*>(entries[i].second);
            }
        }
        throw runtime_error("IoC Container: type not registered!");
    }

    ~IoCContainer() {
        for (int i = (int)deleters.size() - 1; i >= 0; i--) {
            deleters[i]();
        }
    }
};

#endif
