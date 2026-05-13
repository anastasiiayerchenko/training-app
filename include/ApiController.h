#ifndef API_CONTROLLER_H
#define API_CONTROLLER_H

#include <iostream>
#include <string>
#include "httplib.h"
#include "json.hpp"
#include "Service.h"
using namespace std;
using json = nlohmann::json;


class ApiController {
private:
    TrainingService* service;
    httplib::Server server;

public:
    ApiController(TrainingService* srv) { service = srv; }

    void setupRoutes() {
        // CORS-заголовки до кожної відповіді сервера
        server.set_default_headers({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
            {"Access-Control-Allow-Headers", "Content-Type"}
        });

        // OPTIONS-запити
        server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
        });

        // Health check
        server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
            json response = {{"status", "ok"}};
            res.set_content(response.dump(), "application/json");
        });

        // Swagger
        server.Get("/swagger.yml", [](const httplib::Request&, httplib::Response& res) {
            std::ifstream file("swagger.yml");
            if (file.is_open()) {
                std::string content = "";
                std::string line;
                while (std::getline(file, line)) {
                    content += line + "\n";
                }
                file.close();
                res.set_content(content, "text/yaml");
            } else {
                res.status = 404;
                res.set_content("Swagger file not found", "text/plain");
            }
        });

        // POST /athletes — register new athlete
        server.Post("/athletes", [this](const httplib::Request& req, httplib::Response& res) {
            try {
                json body = json::parse(req.body);

                if (!body.contains("name") || !body.contains("age")) {
                    json err = {{"error", "Missing required fields: name, age"}};
                    res.status = 400;
                    res.set_content(err.dump(), "application/json");
                    return;
                }

                string name = body["name"].get<string>();
                int age = body["age"].get<int>();

                int id = service->registerAthlete(name, age);
                json response = {{"id", id}, {"name", name}, {"age", age}};
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

        // GET /athletes — list all athletes
        server.Get("/athletes", [this](const httplib::Request&, httplib::Response& res) {
            vector<AthleteData> athletes = service->getAllAthletes();
            json arr = json::array();
            for (const auto& a : athletes) {
                arr.push_back({{"id", a.id}, {"name", a.name}});
            }
            res.set_content(arr.dump(), "application/json");
        });

        // GET /athletes/:id — get athlete by ID
        server.Get(R"(/athletes/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
            int id = stoi(req.matches[1]);
            string name = service->getAthleteName(id);

            if (name.empty()) {
                json err = {{"error", "Athlete not found"}};
                res.status = 404;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json response = {{"id", id}, {"name", name}};
            res.set_content(response.dump(), "application/json");
        });

        // POST /athletes/:id/trainings — create training for athlete
        server.Post(R"(/athletes/(\d+)/trainings)", [this](const httplib::Request& req, httplib::Response& res) {
            try {
                int athlete_id = stoi(req.matches[1]);
                string name = service->getAthleteName(athlete_id);

                if (name.empty()) {
                    json err = {{"error", "Athlete not found"}};
                    res.status = 404;
                    res.set_content(err.dump(), "application/json");
                    return;
                }

                json body = json::parse(req.body);

                if (!body.contains("type") || !body.contains("date") || !body.contains("exp")) {
                    json err = {{"error", "Missing required fields: type, date, exp"}};
                    res.status = 400;
                    res.set_content(err.dump(), "application/json");
                    return;
                }

                string type = body["type"].get<string>();
                string date = body["date"].get<string>();
                int exp = body["exp"].get<int>();
                string muscle = body.value("muscle", "-");

                service->createTraining(athlete_id, type, muscle, date, exp);

                Training* tr;
                if (type == "Strength") tr = new StrengthTraining(date, muscle);
                else tr = new Training(date, type);

                service->fillTrainingData(tr, muscle, exp);

                json response = {
                    {"status", "created"},
                    {"athlete", name},
                    {"training", tr->toJson()}
                };

                delete tr;

                res.status = 201;
                res.set_content(response.dump(), "application/json");

            } catch (const json::exception& e) {
                json err = {{"error", "Invalid JSON body"}};
                res.status = 400;
                res.set_content(err.dump(), "application/json");
            }
        });

        // GET /athletes/:id/trainings — get training history
        server.Get(R"(/athletes/(\d+)/trainings)", [this](const httplib::Request& req, httplib::Response& res) {
            int athlete_id = stoi(req.matches[1]);
            string name = service->getAthleteName(athlete_id);

            if (name.empty()) {
                json err = {{"error", "Athlete not found"}};
                res.status = 404;
                res.set_content(err.dump(), "application/json");
                return;
            }

            vector<Training*> history = service->getTrainings(athlete_id);
            json arr = json::array();
            for (size_t i = 0; i < history.size(); i++) {
                arr.push_back(history[i]->toJson());
                delete history[i];
            }

            json response = {{"athlete", name}, {"trainings", arr}};
            res.set_content(response.dump(), "application/json");
        });
    }

    void start(int port = 8080) {
        cout << "\n\tPersonalized Training System - Microservice\n";
        cout << "Server listening on http://0.0.0.0:" << port << "\n";
        cout << "Endpoints:\n";
        cout << "  GET  /health\n";
        cout << "  POST /athletes\n";
        cout << "  GET  /athletes\n";
        cout << "  GET  /athletes/:id\n";
        cout << "  POST /athletes/:id/trainings\n";
        cout << "  GET  /athletes/:id/trainings\n\n";
        server.listen("0.0.0.0", port);
    }
};

#endif
