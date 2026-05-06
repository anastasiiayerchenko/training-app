#ifndef SERVICE_H
#define SERVICE_H

#include <string>
#include <vector>
#include <stdexcept>
#include <functional>
#include "Database.h"
using namespace std;


class TrainingService {
private:
    IDatabase* db; 
public:
    TrainingService(IDatabase* database) { db = database; }

    int registerAthlete(string name, int age) {
        if (name == "") throw invalid_argument("Name cannot be empty!");
        if (age < 5 || age > 100) throw invalid_argument("Invalid age!");
        return db->addAthlete(name, age);
    }

    void createTraining(int athlete_id, string type, string muscle, string date, int exp) {
        db->addTraining(athlete_id, type, muscle, date, exp);
    }
    void fillTrainingData(Training* tr, string muscle, int exp) { db->fillTraining(tr, muscle, exp); }
    void showAthletes() { db->showAllAthletes(); }
    vector<Training*> getTrainings(int id) { return db->getAthleteTrainings(id); }
    string getAthleteName(int id) { return db->getAthleteName(id); }
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