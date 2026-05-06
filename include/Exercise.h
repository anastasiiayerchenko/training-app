#ifndef EXERCISE_H
#define EXERCISE_H

#include <iostream>
#include <string>
using namespace std;


class IExercise {
public:
    virtual string getName() const = 0;
    virtual void printDetails(ostream& out) const = 0; 
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