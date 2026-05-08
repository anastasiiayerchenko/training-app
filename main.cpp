#include "include/App.h"
#include "include/ApiController.h"


class MockDatabase : public IDatabase {
private:
    vector<string> names;
public:
    int addAthlete(string name, int age) override {
        names.push_back(name);
        return (int)names.size(); 
    }
    string getAthleteName(int id) override {
        if (id < 1 || id > (int)names.size()) return "";
        return names[id - 1];
    }
    void addTraining(int athlete_id, string type, string muscle, string date, int exp) override {}
    void fillTraining(Training* tr, string muscle, int exp) override {}
    vector<AthleteData> getAllAthletes() override { return vector<AthleteData>(); }
    vector<Training*> getAthleteTrainings(int athlete_id) override { return vector<Training*>(); }
};

void runUnitTests() {
    cout << "    UNIT TESTS \n";
 
    cout << "[TEST] Positive: Registering an athlete... ";
    MockDatabase* mockDb1 = new MockDatabase();
    TrainingService service1(mockDb1);
    int id = service1.registerAthlete("Nazar", 20); 
    if (service1.getAthleteName(id) == "Nazar") cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb1;

    cout << "[TEST] Negative: Empty athlete name... ";
    MockDatabase* mockDb2 = new MockDatabase();
    TrainingService service2(mockDb2); 
    bool errorCaught = false;
    try {
        service2.registerAthlete("", 20); 
    } catch (const invalid_argument& e) {
        errorCaught = true;
    }
    if (errorCaught) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb2;
    
    cout << "[TEST] Negative: Age too young (age=3)... ";
    MockDatabase* mockDb3 = new MockDatabase();
    TrainingService service3(mockDb3);
    errorCaught = false;
    try {
        service3.registerAthlete("Child", 3);
    } catch (const invalid_argument& e) {
        errorCaught = true;
    }
    if (errorCaught) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb3;

    cout << "[TEST] Negative: Age too old (age=150)... ";
    MockDatabase* mockDb4 = new MockDatabase();
    TrainingService service4(mockDb4);
    errorCaught = false;
    try {
        service4.registerAthlete("Elder", 150);
    } catch (const invalid_argument& e) {
        errorCaught = true;
    }
    if (errorCaught) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb4;

    cout << "[TEST] Positive: ExerciseLibrary creates exercise... ";
    int before = ExerciseLibrary::totalExercises;
    IExercise* ex = ExerciseLibrary::createExercise("Strength", "Bench Press", 3, 10, "-");
    if (ExerciseLibrary::totalExercises == before + 1 && ex->getName() == "Bench Press") cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete ex;

    cout << "[TEST] Positive: Training stores exercises... ";
    Training tr("01-01-2026", "Cardio");
    tr.addExercise(ExerciseLibrary::createExercise("Cardio", "Running", 0, 0, "30 min"));
    tr.addExercise(ExerciseLibrary::createExercise("Cardio", "Cycling", 0, 0, "20 min"));
    if (tr.getExerciseCount() == 2 && tr.getExercise(0)->getName() == "Running") cout << "SUCCESS\n";
    else cout << "FAILED\n";

    cout << "[TEST] Positive: StrengthTraining title format... ";
    StrengthTraining st("05-05-2026", "chest");
    if (st.getTitle() == "[05-05-2026] StrengthTraining (chest):") cout << "SUCCESS\n";
    else cout << "FAILED\n";

    cout << "[TEST] Positive: Multiple athletes registration... ";
    MockDatabase* mockDb5 = new MockDatabase();
    TrainingService service5(mockDb5);
    int id1 = service5.registerAthlete("Alice", 25);
    int id2 = service5.registerAthlete("Bob", 30);
    if (id1 != id2 && service5.getAthleteName(id1) == "Alice" && service5.getAthleteName(id2) == "Bob") cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb5;

    cout << "------------------\n";
}


int main() {
    runUnitTests();

    IoCContainer container;
    container.registerInstance<IDatabase>(new SqliteDatabase(Config.dbName));
    container.registerInstance<TrainingService>(new TrainingService(container.resolve<IDatabase>()));

    TrainingService* appService = container.resolve<TrainingService>();

    ApiController api(appService);
    api.setupRoutes();
    api.start(8080);

    return 0;
}