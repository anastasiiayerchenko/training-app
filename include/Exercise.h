#ifndef EXERCISE_H
#define EXERCISE_H

#include <iostream>
#include <string>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;


class IExercise {
public:
    virtual string getName() const = 0;
    virtual void printDetails(ostream& out) const = 0; 
    virtual json toJson() const = 0;
    virtual ~IExercise() {}
};


class StrengthExercise : public IExercise {
private:
    string name;
    int sets, reps;
public:
    StrengthExercise(string n, int s, int r) {
        name = n; sets = s; reps = r;
    }
    string getName() const override { return name; }
    void printDetails(ostream& out) const override { out << sets << "x" << reps; }
    json toJson() const override {
        return json{{"name", name}, {"type", "Strength"}, {"sets", sets}, {"reps", reps}};
    }
};


class CardioExercise : public IExercise {
private:
    string name, duration;
public:
    CardioExercise(string n, string d) {
        name = n; duration = d;
    }
    string getName() const override { return name; }
    void printDetails(ostream& out) const override { out << duration; }
    json toJson() const override {
        return json{{"name", name}, {"type", "Cardio"}, {"duration", duration}};
    }
};


class ExerciseLibrary {
public:
    ExerciseLibrary() = delete;
    inline static int totalExercises = 0; 

    static IExercise* createExercise(string type, string name, int sets, int reps, string duration) {
        totalExercises++;
        if (type == "Cardio") {
            return new CardioExercise(name, duration);
        } else {
            return new StrengthExercise(name, sets, reps);
        }
    }
};

#endif