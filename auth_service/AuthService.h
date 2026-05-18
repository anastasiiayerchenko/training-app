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

    int registerUser(string first_name, string last_name, string username, string dob, string password) {
        if (first_name.empty() || last_name.empty() || username.empty() || password.empty() || dob.empty()) {
            throw invalid_argument("All fields are required");
        }
        if (db->isUsernameTaken(username)) {
            throw invalid_argument("Username is already taken");
        }
        return db->registerUser(first_name, last_name, username, dob, password);
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

    bool changePassword(int id, string old_password, string new_password) {
        if (old_password.empty() || new_password.empty()) {
            throw invalid_argument("Both old and new passwords are required");
        }
        if (new_password.size() < 4) {
            throw invalid_argument("New password must be at least 4 characters");
        }
        bool changed = db->changePassword(id, old_password, new_password);
        if (!changed) {
            throw invalid_argument("Current password is incorrect");
        }
        return true;
    }

    bool deleteAccount(int id) {
        return db->deleteAccount(id);
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
