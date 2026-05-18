#include "RecordApiController.h"

class MockRecordDatabase : public IRecordDatabase {
private:
    vector<Record> data;
    int nextId = 1;
public:
    int addEntry(int athlete_id, string exercise_name, double weight, string date) override {
        Record r(nextId, athlete_id, exercise_name, weight, date);
        data.push_back(r);
        return nextId++;
    }

    vector<Record> getHistory(int athlete_id, string exercise_name) override {
        vector<Record> result;
        for (const auto& r : data) {
            if (r.athlete_id == athlete_id && r.exercise_name == exercise_name) {
                result.push_back(r);
            }
        }
        return result;
    }

    vector<BestRecord> getBestRecords(int athlete_id) override {
        map<string, BestRecord> best;
        for (const auto& r : data) {
            if (r.athlete_id == athlete_id) {
                if (best.find(r.exercise_name) == best.end() || r.weight > best[r.exercise_name].best_weight) {
                    best[r.exercise_name] = {r.exercise_name, r.weight, r.date};
                }
            }
        }
        vector<BestRecord> result;
        for (const auto& pair : best) result.push_back(pair.second);
        return result;
    }

    bool deleteEntry(int id) override {
        for (size_t i = 0; i < data.size(); i++) {
            if (data[i].id == id) {
                data.erase(data.begin() + i);
                return true;
            }
        }
        return false;
    }

    void deleteAthleteEntries(int athlete_id) override {
        vector<Record> remaining;
        for (const auto& r : data) {
            if (r.athlete_id != athlete_id) remaining.push_back(r);
        }
        data = remaining;
    }

    vector<string> getRecordedExercises(int athlete_id) override {
        set<string> unique_ex;
        for (const auto& r : data) {
            if (r.athlete_id == athlete_id) {
                unique_ex.insert(r.exercise_name);
            }
        }
        return vector<string>(unique_ex.begin(), unique_ex.end());
    }
};

void runUnitTests() {
    cout << "    RECORDS SERVICE - UNIT TESTS\n";

    cout << "[TEST] Positive: Adding weight entry... ";
    MockRecordDatabase* mockDb1 = new MockRecordDatabase();
    RecordService service1(mockDb1);
    int id = service1.addEntry(1, "Bench Press", 60.0, "2026-05-17");
    if (id == 1) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb1;

    cout << "[TEST] Negative: Empty exercise name... ";
    MockRecordDatabase* mockDb2 = new MockRecordDatabase();
    RecordService service2(mockDb2);
    bool errorCaught = false;
    try {
        service2.addEntry(1, "", 60.0, "2026-05-17");
    } catch (const invalid_argument& e) {
        errorCaught = true;
    }
    if (errorCaught) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb2;

    cout << "[TEST] Negative: Zero weight... ";
    MockRecordDatabase* mockDb3 = new MockRecordDatabase();
    RecordService service3(mockDb3);
    errorCaught = false;
    try {
        service3.addEntry(1, "Bench Press", 0, "2026-05-17");
    } catch (const invalid_argument& e) {
        errorCaught = true;
    }
    if (errorCaught) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb3;

    cout << "[TEST] Positive: Get history... ";
    MockRecordDatabase* mockDb4 = new MockRecordDatabase();
    RecordService service4(mockDb4);
    service4.addEntry(1, "Bench Press", 40.0, "2026-03-01");
    service4.addEntry(1, "Bench Press", 50.0, "2026-03-15");
    service4.addEntry(1, "Squat", 80.0, "2026-03-01");
    vector<Record> history = service4.getHistory(1, "Bench Press");
    if (history.size() == 2 && history[0].weight == 40.0) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb4;

    cout << "[TEST] Positive: Best records... ";
    MockRecordDatabase* mockDb5 = new MockRecordDatabase();
    RecordService service5(mockDb5);
    service5.addEntry(1, "Bench Press", 40.0, "2026-03-01");
    service5.addEntry(1, "Bench Press", 60.0, "2026-03-15");
    service5.addEntry(1, "Bench Press", 45.0, "2026-04-01");
    service5.addEntry(1, "Squat", 100.0, "2026-03-01");
    vector<BestRecord> records = service5.getBestRecords(1);
    if (records.size() == 2) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb5;

    cout << "[TEST] Positive: Delete entry... ";
    MockRecordDatabase* mockDb6 = new MockRecordDatabase();
    RecordService service6(mockDb6);
    int entryId = service6.addEntry(1, "Bench Press", 60.0, "2026-05-17");
    if (service6.deleteEntry(entryId)) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb6;

    cout << "[TEST] Negative: Delete non-existent... ";
    MockRecordDatabase* mockDb7 = new MockRecordDatabase();
    RecordService service7(mockDb7);
    if (!service7.deleteEntry(999)) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb7;

    cout << "------------------\n";
}

int main() {
    runUnitTests();

    string trainingUrl = "http://localhost:8080";
    char* envUrl = getenv("TRAINING_SERVICE_URL");
    if (envUrl) trainingUrl = string(envUrl);

    string authUrl = "http://localhost:8082";
    char* envAuth = getenv("AUTH_SERVICE_URL");
    if (envAuth) authUrl = string(envAuth);

    IoCContainer container;
    container.registerInstance<IRecordDatabase>(new SqliteRecordDatabase("db/records.db"));
    container.registerInstance<RecordService>(new RecordService(container.resolve<IRecordDatabase>(), trainingUrl));

    RecordService* appService = container.resolve<RecordService>();

    RecordApiController api(appService, trainingUrl, authUrl);
    api.setupRoutes();
    api.start(8081);

    return 0;
}
