#include "App.h"
#include "ApiController.h"


class MockDatabase : public IDatabase {
public:
    void addTraining(int athlete_id, string type, string muscle, string date, int exp) override {}
    void fillTraining(Training* tr, string muscle, int exp) override {}
    vector<Training*> getAthleteTrainings(int athlete_id) override { return vector<Training*>(); }
};

void runUnitTests() {
    cout << "    UNIT TESTS \n";
 


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



    cout << "------------------\n";
}


int main() {
    runUnitTests();

    IoCContainer container;
    container.registerInstance<IDatabase>(new SqliteDatabase(Config.dbName));
    string authUrl = "http://localhost:8082";
    char* envAuth = getenv("AUTH_SERVICE_URL");
    if (envAuth) authUrl = string(envAuth);
    container.registerInstance<TrainingService>(new TrainingService(container.resolve<IDatabase>(), authUrl));

    TrainingService* appService = container.resolve<TrainingService>();

    ApiController api(appService);
    api.setupRoutes();
    api.start(8080);

    return 0;
}