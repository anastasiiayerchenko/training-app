# Training App — Microservice

Personalized Training System — C++ REST microservice with SQLite database, IoC/DI architecture, CI/CD pipeline and Docker deployment.

## Features

- Register athletes and create personalized training programs via REST API
- Strength, Cardio, and Fullbody training types
- Exercise generation based on experience level and muscle group
- Training history tracking
- SQLite database for persistent storage
- JSON API responses
- Health check endpoint

## Architecture

- **Microservice** — HTTP REST API with JSON responses (cpp-httplib)
- **IoC Container** — generic template-based dependency injection container
- **DI** — constructor injection via `IDatabase` interface
- **Unit Tests** — 8 tests (positive + negative scenarios) with MockDatabase

## API Endpoints

| Method | URL | Description |
|--------|-----|-------------|
| `GET` | `/health` | Health check |
| `POST` | `/athletes` | Register new athlete |
| `GET` | `/athletes` | List all athletes |
| `GET` | `/athletes/:id` | Get athlete by ID |
| `POST` | `/athletes/:id/trainings` | Create training |
| `GET` | `/athletes/:id/trainings` | Get training history |

### Examples

```bash
# Health check
curl http://localhost:8080/health

# Register athlete
curl -X POST http://localhost:8080/athletes \
  -H "Content-Type: application/json" \
  -d '{"name": "Nazar", "age": 20}'

# List all athletes
curl http://localhost:8080/athletes

# Create strength training
curl -X POST http://localhost:8080/athletes/1/trainings \
  -H "Content-Type: application/json" \
  -d '{"type": "Strength", "muscle": "chest", "date": "01-01-2026", "exp": 2}'

# Get training history
curl http://localhost:8080/athletes/1/trainings
```

## Project Structure

```
├── main.cpp                 # Entry point + unit tests
├── include/                 # Header files
│   ├── Exercise.h           # IExercise, StrengthExercise, CardioExercise
│   ├── Training.h           # Training, StrengthTraining
│   ├── Database.h           # IDatabase, SqliteDatabase
│   ├── Service.h            # TrainingService, IoCContainer
│   ├── ApiController.h      # REST API controller (HTTP endpoints)
│   ├── App.h                # Config
│   ├── httplib.h            # HTTP server library (header-only)
│   └── json.hpp             # JSON library (header-only)
├── db/                      # Database files
│   ├── athletes.db
│   ├── sqlite3.c
│   └── sqlite3.h
├── .github/workflows/
│   └── ci.yml               # CI/CD pipeline
├── Dockerfile
└── docker-compose.yml       # Blue-Green deployment
```

## Build & Run

```bash
# Compile SQLite (once)
gcc -c db/sqlite3.c -o db/sqlite3.o

# Build
g++ -std=c++17 main.cpp db/sqlite3.o -o app -lpthread -ldl

# Run (starts HTTP server on port 8080)
./app
```

## Docker

```bash
# Build and run with Docker
docker build -t training-app .
docker run -p 8080:8080 training-app

# Blue-Green deployment
docker-compose up app-blue          # start blue on :8080
docker-compose --profile green up   # start green on :8081
```

## CI/CD Pipeline

The GitHub Actions pipeline runs on every push to `main`:

1. **Build** — compiles the application
2. **Test** — runs unit tests + API integration tests (curl)
3. **Docker** — builds image and verifies health check

## Deployment Strategies

- **Recreate**: `docker-compose up --force-recreate`
- **Blue-Green**: switch between `app-blue` (:8080) and `app-green` (:8081) services
- **Rolling Update**: via `docker-compose up --no-deps --build app-blue`
