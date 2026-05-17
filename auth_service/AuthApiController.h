#ifndef AUTH_API_CONTROLLER_H
#define AUTH_API_CONTROLLER_H

#include "AuthService.h"
#include "httplib.h"
#include "json.hpp"
#include <iostream>

using namespace std;
using json = nlohmann::json;

class AuthApiController {
private:
    AuthService* service;
    httplib::Server server;

public:
    AuthApiController(AuthService* srv) {
        service = srv;
    }

    void setupRoutes() {
        server.set_default_headers({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
            {"Access-Control-Allow-Headers", "Content-Type"}
        });

        server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
        });

        server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
            json response = {{"status", "ok"}, {"service", "auth"}};
            res.set_content(response.dump(), "application/json");
        });

        server.Post("/register", [this](const httplib::Request& req, httplib::Response& res) {
            try {
                json body = json::parse(req.body);
                if (!body.contains("first_name") || !body.contains("last_name") || 
                    !body.contains("username") || !body.contains("age") || !body.contains("password")) {
                    json err = {{"error", "Missing fields"}};
                    res.status = 400;
                    res.set_content(err.dump(), "application/json");
                    return;
                }

                string first_name = body["first_name"].get<string>();
                string last_name = body["last_name"].get<string>();
                string username = body["username"].get<string>();
                int age = body["age"].get<int>();
                string password = body["password"].get<string>();

                int id = service->registerUser(first_name, last_name, username, age, password);
                json response = {{"id", id}, {"username", username}, {"message", "Registered successfully"}};
                res.status = 201;
                res.set_content(response.dump(), "application/json");

            } catch (const invalid_argument& e) {
                json err = {{"error", e.what()}};
                res.status = 400;
                res.set_content(err.dump(), "application/json");
            } catch (const json::exception& e) {
                json err = {{"error", "Invalid JSON body"}};
                res.status = 400;
                res.set_content(err.dump(), "application/json");
            }
        });

        server.Post("/login", [this](const httplib::Request& req, httplib::Response& res) {
            try {
                json body = json::parse(req.body);
                if (!body.contains("username") || !body.contains("password")) {
                    json err = {{"error", "Missing fields"}};
                    res.status = 400;
                    res.set_content(err.dump(), "application/json");
                    return;
                }

                string username = body["username"].get<string>();
                string password = body["password"].get<string>();

                int id = service->login(username, password);
                if (id != -1) {
                    json response = {{"id", id}, {"username", username}};
                    res.status = 200;
                    res.set_content(response.dump(), "application/json");
                } else {
                    json err = {{"error", "Invalid credentials"}};
                    res.status = 401;
                    res.set_content(err.dump(), "application/json");
                }

            } catch (const invalid_argument& e) {
                json err = {{"error", e.what()}};
                res.status = 400;
                res.set_content(err.dump(), "application/json");
            } catch (const json::exception& e) {
                json err = {{"error", "Invalid JSON body"}};
                res.status = 400;
                res.set_content(err.dump(), "application/json");
            }
        });

        server.Get(R"(/users/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
            int id = stoi(req.matches[1]);
            UserData u = service->getUser(id);

            if (u.id != -1) {
                json response = {
                    {"id", u.id},
                    {"first_name", u.first_name},
                    {"last_name", u.last_name},
                    {"username", u.username},
                    {"age", u.age}
                };
                res.status = 200;
                res.set_content(response.dump(), "application/json");
            } else {
                json err = {{"error", "User not found"}};
                res.status = 404;
                res.set_content(err.dump(), "application/json");
            }
        });
    }

    void start(int port = 8082) {
        cout << "\n\tAuth Service - Microservice\n";
        cout << "Server listening on http://0.0.0.0:" << port << "\n";
        cout << "Endpoints:\n";
        cout << "  GET  /health\n";
        cout << "  POST /register\n";
        cout << "  POST /login\n";
        cout << "  GET  /users/:id\n\n";
        server.listen("0.0.0.0", port);
    }
};

#endif
