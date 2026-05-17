#ifndef SERVICE_H
#define SERVICE_H

#include <string>
#include <vector>
#include <stdexcept>
#include <functional>
#include "Database.h"
#include "httplib.h"
#include "json.hpp"
using namespace std;
using json = nlohmann::json;


class TrainingService {
private:
    IDatabase* db; 
    string authServiceUrl;
public:
    TrainingService(IDatabase* database, string authUrl = "http://localhost:8082") { 
        db = database; 
        authServiceUrl = authUrl;
    }

    void createTraining(int athlete_id, string type, string muscle, string date, int exp) {
        db->addTraining(athlete_id, type, muscle, date, exp);
    }
    void fillTrainingData(Training* tr, string muscle, int exp) { db->fillTraining(tr, muscle, exp); }
    vector<Training*> getTrainings(int id) { return db->getAthleteTrainings(id); }
    string getAthleteName(int id) {
        httplib::Client cli(authServiceUrl);
        auto res = cli.Get("/users/" + to_string(id));
        if (res && res->status == 200) {
            try {
                json j = json::parse(res->body);
                if (j.contains("first_name") && j.contains("last_name")) {
                    return j["first_name"].get<string>() + " " + j["last_name"].get<string>();
                }
            } catch(...) {}
        }
        return "";
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