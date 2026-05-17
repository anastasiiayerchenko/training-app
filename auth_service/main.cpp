#include "AuthApiController.h"

class MockAuthDatabase : public IAuthDatabase {
private:
    vector<UserData> data;
    vector<string> passwords;
    int nextId = 1;
public:
    int registerUser(string first_name, string last_name, string username, int age, string password) override {
        UserData u = {nextId, first_name, last_name, username, age};
        data.push_back(u);
        passwords.push_back(password);
        return nextId++;
    }

    int authenticate(string username, string password) override {
        for (size_t i = 0; i < data.size(); i++) {
            if (data[i].username == username && passwords[i] == password) {
                return data[i].id;
            }
        }
        return -1;
    }

    bool isUsernameTaken(string username) override {
        for (const auto& u : data) {
            if (u.username == username) return true;
        }
        return false;
    }

    UserData getUser(int id) override {
        for (const auto& u : data) {
            if (u.id == id) return u;
        }
        return {-1, "", "", "", 0};
    }
};

void runUnitTests() {
    cout << "    AUTH SERVICE - UNIT TESTS\n";

    cout << "[TEST] Positive: Register User... ";
    MockAuthDatabase* mockDb1 = new MockAuthDatabase();
    AuthService service1(mockDb1);
    int id = service1.registerUser("Test", "User", "testuser", 20, "pass");
    if (id == 1) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb1;

    cout << "[TEST] Negative: Duplicate Username... ";
    MockAuthDatabase* mockDb2 = new MockAuthDatabase();
    AuthService service2(mockDb2);
    service2.registerUser("Test", "User", "testuser", 20, "pass");
    bool errorCaught = false;
    try {
        service2.registerUser("Test2", "User2", "testuser", 22, "pass2");
    } catch (const invalid_argument& e) {
        errorCaught = true;
    }
    if (errorCaught) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb2;

    cout << "[TEST] Positive: Login... ";
    MockAuthDatabase* mockDb3 = new MockAuthDatabase();
    AuthService service3(mockDb3);
    service3.registerUser("Test", "User", "testuser", 20, "pass");
    int loginId = service3.login("testuser", "pass");
    if (loginId == 1) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb3;

    cout << "[TEST] Negative: Invalid Password... ";
    MockAuthDatabase* mockDb4 = new MockAuthDatabase();
    AuthService service4(mockDb4);
    service4.registerUser("Test", "User", "testuser", 20, "pass");
    int invalidLoginId = service4.login("testuser", "wrongpass");
    if (invalidLoginId == -1) cout << "SUCCESS\n";
    else cout << "FAILED\n";
    delete mockDb4;

    cout << "------------------\n";
}

int main() {
    runUnitTests();

    IoCContainer container;
    container.registerInstance<IAuthDatabase>(new SqliteAuthDatabase("db/users.db"));
    container.registerInstance<AuthService>(new AuthService(container.resolve<IAuthDatabase>()));

    AuthService* appService = container.resolve<AuthService>();

    AuthApiController api(appService);
    api.setupRoutes();
    api.start(8082);

    return 0;
}
