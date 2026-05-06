# Training App

Personalized Training System — C++ console application with SQLite database, IoC/DI architecture, CI/CD pipeline and Docker deployment.

## Features

- Register athletes and create personalized training programs
- Strength, Cardio, and Fullbody training types
- Exercise generation based on experience level and muscle group
- Training history tracking
- SQLite database for persistent storage

## Architecture

- **IoC Container** — generic template-based dependency injection container
- **DI** — constructor injection via `IDatabase` interface
- **Unit Tests** — 8 tests (positive + negative scenarios) with MockDatabase

## Project Structure

```
├── main.cpp              # Entry point + unit tests
├── include/              # Header files
│   ├── Exercise.h        # IExercise, StrengthExercise, CardioExercise
│   ├── Training.h        # Training, StrengthTraining
│   ├── Database.h        # IDatabase, SqliteDatabase
│   ├── Service.h         # TrainingService, IoCContainer
│   └── App.h             # TrainingApp (console UI)
├── db/                   # Database files
│   ├── athletes.db
│   ├── sqlite3.c
│   └── sqlite3.h
├── .github/workflows/
│   └── ci.yml            # CI/CD pipeline
├── Dockerfile
└── docker-compose.yml    # Blue-Green deployment
```

## Build & Run

```bash
# Compile SQLite (once)
gcc -c db/sqlite3.c -o db/sqlite3.o

# Build
g++ -std=c++17 main.cpp db/sqlite3.o -o app -lpthread -ldl

# Run
./app
```

## Docker

```bash
# Build and run with Docker
docker build -t training-app .
docker run -it training-app

# Blue-Green deployment
docker-compose up app-blue          # start blue
docker-compose --profile green up   # switch to green
```

## CI/CD Pipeline

The GitHub Actions pipeline runs on every push to `main`:

1. **Build** — compiles the application
2. **Test** — runs unit tests, fails if any test fails
3. **Docker** — builds and verifies the Docker image

## Deployment Strategies

- **Recreate**: `docker-compose up --force-recreate`
- **Blue-Green**: switch between `app-blue` and `app-green` services
- **Rolling Update**: via `docker-compose up --no-deps --build app-blue`
