#ifndef RECORD_API_CONTROLLER_H
#define RECORD_API_CONTROLLER_H

#include "RecordService.h"
#include "httplib.h"
#include "json.hpp"
#include <iostream>
#include <string>
using namespace std;
using json = nlohmann::json;

class RecordApiController {
private:
    RecordService* service;
    httplib::Server server;
    string trainingServiceUrl;
    string authServiceUrl;

    bool athleteExists(int athlete_id) {
        httplib::Client cli(authServiceUrl);
        auto res = cli.Get("/users/" + to_string(athlete_id));
        return res && res->status == 200;
    }

public:
    RecordApiController(RecordService* srv, string tsUrl = "http://localhost:8080", string authUrl = "http://localhost:8082") {
        service = srv;
        trainingServiceUrl = tsUrl;
        authServiceUrl = authUrl;
    }

    void setupRoutes() {
        server.set_default_headers({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS"},
            {"Access-Control-Allow-Headers", "Content-Type"}
        });

        server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
        });

        server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
            json response = {{"status", "ok"}, {"service", "records"}};
            res.set_content(response.dump(), "application/json");
        });

        // POST /entries — додати запис ваги
        server.Post("/entries", [this](const httplib::Request& req, httplib::Response& res) {
            try {
                json body = json::parse(req.body);

                if (!body.contains("athlete_id") || !body.contains("exercise_name")
                    || !body.contains("weight") || !body.contains("date")) {
                    json err = {{"error", "Missing fields: athlete_id, exercise_name, weight, date"}};
                    res.status = 400;
                    res.set_content(err.dump(), "application/json");
                    return;
                }

                int athlete_id = body["athlete_id"].get<int>();
                string exercise_name = body["exercise_name"].get<string>();
                double weight = body["weight"].get<double>();
                string date = body["date"].get<string>();

                if (!athleteExists(athlete_id)) {
                    json err = {{"error", "Athlete not found in Auth Service"}};
                    res.status = 404;
                    res.set_content(err.dump(), "application/json");
                    return;
                }

                int id = service->addEntry(athlete_id, exercise_name, weight, date);
                json response = {{"id", id}, {"athlete_id", athlete_id},
                    {"exercise", exercise_name}, {"weight", weight}, {"date", date}};
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

        // GET /entries/:athlete_id/:exercise — історія ваг на вправу
        server.Get(R"(/entries/(\d+)/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
            int athlete_id = stoi(req.matches[1]);
            string exercise = req.matches[2];

            vector<Record> history = service->getHistory(athlete_id, exercise);
            json arr = json::array();
            for (const auto& r : history) {
                arr.push_back(r.toJson());
            }

            json response = {{"athlete_id", athlete_id}, {"exercise", exercise}, {"history", arr}};
            res.set_content(response.dump(), "application/json");
        });

        // GET /records/:athlete_id — рекорди атлета (MAX по кожній вправі)
        server.Get(R"(/records/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
            int athlete_id = stoi(req.matches[1]);

            vector<BestRecord> records = service->getBestRecords(athlete_id);
            json arr = json::array();
            for (const auto& r : records) {
                arr.push_back(r.toJson());
            }

            json response = {{"athlete_id", athlete_id}, {"records", arr}};
            res.set_content(response.dump(), "application/json");
        });

        // DELETE /entries/:id — видалити запис
        server.Delete(R"(/entries/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
            int id = stoi(req.matches[1]);
            bool deleted = service->deleteEntry(id);

            if (deleted) {
                json response = {{"status", "deleted"}, {"id", id}};
                res.set_content(response.dump(), "application/json");
            } else {
                json err = {{"error", "Entry not found"}};
                res.status = 404;
                res.set_content(err.dump(), "application/json");
            }
        });

        // GET /available_exercises/:athlete_id — список доступних вправ
        server.Get(R"(/available_exercises/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
            int athlete_id = stoi(req.matches[1]);

            if (!athleteExists(athlete_id)) {
                json err = {{"error", "Athlete not found in Auth Service"}};
                res.status = 404;
                res.set_content(err.dump(), "application/json");
                return;
            }

            auto list = service->getAvailableExercises(athlete_id);
            json arr = json::array();
            for (const auto& ex : list) {
                arr.push_back(ex);
            }

            json response = {{"athlete_id", athlete_id}, {"exercises", arr}};
            res.set_content(response.dump(), "application/json");
        });
    }

    void start(int port = 8081) {
        cout << "\n\tRecords Service - Microservice\n";
        cout << "Server listening on http://0.0.0.0:" << port << "\n";
        cout << "Training Service at: " << trainingServiceUrl << "\n";
        cout << "Auth Service at: " << authServiceUrl << "\n";
        cout << "Endpoints:\n";
        cout << "  GET    /health\n";
        cout << "  POST   /entries\n";
        cout << "  GET    /entries/:athlete_id/:exercise\n";
        cout << "  GET    /records/:athlete_id\n";
        cout << "  GET    /available_exercises/:athlete_id\n";
        cout << "  DELETE /entries/:id\n\n";
        server.listen("0.0.0.0", port);
    }
};

#endif
