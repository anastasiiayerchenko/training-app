#ifndef API_CONTROLLER_H
#define API_CONTROLLER_H

#include "Service.h"
#include "httplib.h"
#include "json.hpp"
#include <iostream>
#include <string>
using namespace std;
using json = nlohmann::json;

class ApiController {
private:
  TrainingService *service;
  httplib::Server server;

public:
  ApiController(TrainingService *srv) { service = srv; }

  void setupRoutes() {
    // CORS-заголовки до кожної відповіді сервера
    server.set_default_headers(
        {{"Access-Control-Allow-Origin", "*"},
         {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
         {"Access-Control-Allow-Headers", "Content-Type"}});

    // OPTIONS-запити
    server.Options(".*", [](const httplib::Request &, httplib::Response &res) {
      res.status = 200;
    });

    // Health check
    server.Get("/health", [](const httplib::Request &, httplib::Response &res) {
      json response = {{"status", "ok"}};
      res.set_content(response.dump(), "application/json");
    });

    // Swagger
    server.Get("/swagger.yml",
               [](const httplib::Request &, httplib::Response &res) {
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


    // GET /athletes/:id — get athlete by ID
    server.Get(R"(/athletes/(\d+))",
               [this](const httplib::Request &req, httplib::Response &res) {
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
    server.Post(R"(/athletes/(\d+)/trainings)", [this](
                                                    const httplib::Request &req,
                                                    httplib::Response &res) {
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

        if (!body.contains("type") || !body.contains("date") ||
            !body.contains("exp")) {
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

        Training *tr;
        if (type == "Strength")
          tr = new StrengthTraining(date, muscle);
        else
          tr = new Training(date, type);

        service->fillTrainingData(tr, muscle, exp);

        json response = {{"status", "created"},
                         {"athlete", name},
                         {"training", tr->toJson()}};

        delete tr;

        res.status = 201;
        res.set_content(response.dump(), "application/json");

      } catch (const json::exception &e) {
        json err = {{"error", "Invalid JSON body"}};
        res.status = 400;
        res.set_content(err.dump(), "application/json");
      }
    });

    // GET /athletes/:id/trainings — get training history
    server.Get(R"(/athletes/(\d+)/trainings)",
               [this](const httplib::Request &req, httplib::Response &res) {
                 int athlete_id = stoi(req.matches[1]);
                 string name = service->getAthleteName(athlete_id);

                 if (name.empty()) {
                   json err = {{"error", "Athlete not found"}};
                   res.status = 404;
                   res.set_content(err.dump(), "application/json");
                   return;
                 }

                 vector<Training *> history = service->getTrainings(athlete_id);
                 json arr = json::array();
                 for (size_t i = 0; i < history.size(); i++) {
                   arr.push_back(history[i]->toJson());
                   delete history[i];
                 }

                 json response = {{"athlete", name}, {"trainings", arr}};
                 res.set_content(response.dump(), "application/json");
               });

    // DELETE /athletes/:id/trainings — delete all trainings for an athlete
    server.Delete(R"(/athletes/(\d+)/trainings)",
               [this](const httplib::Request &req, httplib::Response &res) {
                 int athlete_id = stoi(req.matches[1]);
                 service->deleteAthleteTrainings(athlete_id);
                 json response = {{"message", "All trainings deleted"}, {"athlete_id", athlete_id}};
                 res.status = 200;
                 res.set_content(response.dump(), "application/json");
               });
  }

  void start(int port = 8080) {
    cout << "\n\tPersonalized Training System - Microservice\n";
    cout << "Server listening on http://0.0.0.0:" << port << "\n";
    cout << "Endpoints:\n";
    cout << "  GET    /health\n";
    cout << "  GET    /athletes/:id\n";
    cout << "  POST   /athletes/:id/trainings\n";
    cout << "  GET    /athletes/:id/trainings\n";
    cout << "  DELETE /athletes/:id/trainings\n\n";
    server.listen("0.0.0.0", port);
  }
};

#endif
