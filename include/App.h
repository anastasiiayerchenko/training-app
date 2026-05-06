#ifndef APP_H
#define APP_H

#include <iostream>
#include <string>
#include "Service.h"
using namespace std;

const class {
public:
    const string dbName = "db/athletes.db";
    const string appName = "Personalized Training System";
} Config;


class TrainingApp {
private:
    TrainingService* service;

    int getValidInt() {
        int val;
        while (!(cin >> val)) {
            cout << "Invalid input! Try again: ";
            cin.clear();
            while (cin.get() != '\n'); 
        }
        return val;
    }

    string askMuscle() {
        string muscle;
        while (true) {
            cout << "Which muscle group? (chest/arms/back/legs): ";
            cin >> muscle;
            for (size_t i = 0; i < muscle.length(); i++) muscle[i] = tolower(muscle[i]);
            if (muscle == "chest" || muscle == "arms" || muscle == "back" || muscle == "legs") return muscle;
            cout << "Invalid group!\n";
        }
    }

    void addAthleteMenu() {
        string name;
        int age;
        cout << "\nEnter athlete name: "; cin >> name;
        cout << "Enter age: "; age = getValidInt();
        
        int athlete_id;
        try {
            athlete_id = service->registerAthlete(name, age);
        } catch (const invalid_argument& e) {
            cout << "Error: " << e.what() << endl;
            return;
        }
        
        cout << "\n\tTraining menu for " << name << endl;
        cout << "Create training now? (1 - Yes, 2 - No): ";
        int choice = getValidInt();
        if (choice == 1) createTrainingFor(athlete_id, name);
    }

    void createTrainingFor(int athlete_id, string name) {
        int goalChoice, exp;
        string date;
        
        cout << "\nGoals:\n1. Weight loss (Cardio)\n2. Muscle gain (Strength)\n3. Maintaining fitness\nChoice: ";
        goalChoice = getValidInt();

        cout << "Experience:\n1. Beginner\n2. Intermediate\n3. Advanced\nChoice: ";
        exp = getValidInt();

        cout << "Date (DD-MM-YYYY): ";
        cin >> date;

        string db_type, db_muscle = "-";
        if (goalChoice == 1) db_type = "Cardio";
        else if (goalChoice == 2) {
            db_type = "Strength";
            db_muscle = askMuscle();
        }
        else db_type = "Fullbody";

        service->createTraining(athlete_id, db_type, db_muscle, date, exp);

        cout << "\nGenerated Exercises for " << name << ":\n";
        Training* tr;
        if (db_type == "Strength") tr = new StrengthTraining(date, db_muscle);
        else tr = new Training(date, db_type);
        
        service->fillTrainingData(tr, db_muscle, exp);
        tr->show();
        delete tr;
        
        cout << "\nTraining saved successfully!\n";
    }

    void addTrainingToExistingMenu() {
        cout << "\nSelect athlete to add training to:\n";
        service->showAthletes();
        cout << "Enter athlete's ID: ";
        int id = getValidInt();
        string name = service->getAthleteName(id);
        
        if (name != "") {
            cout << "\n\tAdding new training for " << name << endl;
            createTrainingFor(id, name);
        } else {
            cout << "Athlete not found!\n";
        }
    }

    void showTrainingsMenu() {
        cout << "\nEnter athlete's ID to view trainings: ";
        int id = getValidInt();
        string name = service->getAthleteName(id);

        if (name != "") {
            vector<Training*> history = service->getTrainings(id);
            if (history.empty()) {
                cout << "No trainings recorded yet for " << name << ".\n";
                return;
            }
            cout << "\nTrainings for " << name << ":\n";
            for (size_t i = 0; i < history.size(); i++) {
                history[i]->show();
                delete history[i];
            }
        } else {
            cout << "Athlete not found!\n";
        }
    }

    void showAllAthletes() {
        cout << "\nList of athletes:\n";
        service->showAthletes();
    }

public:
    TrainingApp(TrainingService* srv) { service = srv; }

    void run() {
        cout << "\n\t" << Config.appName << "\n";
        cout << "Welcome! This program can create a personalized training for you!\n";

        while (true) {
            cout << "\n\tMAIN MENU\n";
            cout << "1. Add new athlete\n";
            cout << "2. Add training to existing athlete\n";
            cout << "3. Show athlete's trainings\n";
            cout << "4. Show all athletes\n";
            cout << "5. Exit\n";
            cout << "Choice: ";
            int choice = getValidInt();

            if (choice == 1) addAthleteMenu();
            else if (choice == 2) addTrainingToExistingMenu();
            else if (choice == 3) showTrainingsMenu();
            else if (choice == 4) showAllAthletes();
            else if (choice == 5) break;
            else cout << "Invalid choice!\n";
        }
    }
};

#endif