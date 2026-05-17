#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include "AuthDatabase.h"
#include <stdexcept>
#include <vector>
#include <functional>

class AuthService {
private:
    IAuthDatabase* db;

public:
    AuthService(IAuthDatabase* database) { db = database; }

    int registerUser(string first_name, string last_name, string username, int age, string password) {
        if (first_name.empty() || last_name.empty() || username.empty() || password.empty()) {
            throw invalid_argument("All fields are required");
        }
        if (age < 5 || age > 120) {
            throw invalid_argument("Invalid age");
        }
        if (db->isUsernameTaken(username)) {
            throw invalid_argument("Username is already taken");
        }
        return db->registerUser(first_name, last_name, username, age, password);
    }

    int login(string username, string password) {
        if (username.empty() || password.empty()) {
            throw invalid_argument("Username and password are required");
        }
        return db->authenticate(username, password);
    }

    UserData getUser(int id) {
        return db->getUser(id);
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
