#ifndef TRAINING_H
#define TRAINING_H

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include "Exercise.h"
using namespace std;


class Training {
protected:
    string date, type;
    vector<IExercise*> exercises;
public:
    Training(string d, string t) {
        date = d; type = t;
    }
    virtual string getTitle() const {
        return "[" + date + "] " + type + ":";
    }
    void show() const {
        cout << "\n" << getTitle() << endl;
        for (size_t i = 0; i < exercises.size(); i++) {
            cout << " - " << left << setw(25) << exercises[i]->getName() << " | ";
            exercises[i]->printDetails(cout);
            cout << endl;
        }
    }
    void addExercise(IExercise* e) { exercises.push_back(e); }
    string getType() const { return type; }
    string getDate() const { return date; }
    size_t getExerciseCount() const { return exercises.size(); }
    IExercise* getExercise(size_t index) const { return exercises[index]; }

    virtual json toJson() const {
        json j;
        j["date"] = date;
        j["type"] = type;
        json exArr = json::array();
        for (size_t i = 0; i < exercises.size(); i++) {
            exArr.push_back(exercises[i]->toJson());
        }
        j["exercises"] = exArr;
        return j;
    }
    
    virtual ~Training() {
        for (size_t i = 0; i < exercises.size(); i++) {
            delete exercises[i];
        }
    }
};


class StrengthTraining : public Training {
private:
    string muscle;
public:
    StrengthTraining(string d, string m) : Training(d, "Strength") { muscle = m; }
    string getTitle() const override { return "[" + date + "] StrengthTraining (" + muscle + "):"; }
    json toJson() const override {
        json j = Training::toJson();
        j["muscle"] = muscle;
        return j;
    }
};

#endif