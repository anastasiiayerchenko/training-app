# Training System — Microservices Architecture

Personalized Training System — A distributed C++ REST microservice architecture with SQLite databases, IoC/DI, CI/CD pipelines, and Docker Blue-Green deployment.

## Architecture Overview

The system has been refactored from a monolith into three independent microservices:

1. **Auth Service (Port 8082)** — Manages user registration, authentication, and user profiles.
2. **Training Service (Port 8080)** — Generates and tracks personalized training programs.
3. **Records Service (Port 8081)** — Tracks best exercise weights and training history.

Each microservice uses:
- **HTTP REST API** with JSON responses (`cpp-httplib` + `json.hpp`)
- **IoC Container** — generic template-based dependency injection
- **SQLite Database** — separate database file per microservice

## API Endpoints

### 1. Auth Service (Port 8082)
| Method | URL | Description |
|--------|-----|-------------|
| `GET` | `/health` | Health check |
| `POST` | `/register` | Register a new user |
| `POST` | `/login` | Authenticate user |
| `GET` | `/users/:id` | Get user info |

### 2. Training Service (Port 8080)
| Method | URL | Description |
|--------|-----|-------------|
| `GET` | `/health` | Health check |
| `GET` | `/athletes/:id` | Get athlete name via Auth Service |
| `POST` | `/athletes/:id/trainings` | Create new training session |
| `GET` | `/athletes/:id/trainings` | Get training history |

### 3. Records Service (Port 8081)
| Method | URL | Description |
|--------|-----|-------------|
| `GET` | `/health` | Health check |
| `POST` | `/entries` | Add weight record for an exercise |
| `GET` | `/entries/:athlete_id/:exercise` | History of weights for an exercise |
| `GET` | `/records/:athlete_id` | Best records per exercise |
| `GET` | `/available_exercises/:athlete_id`| List of exercises performed |
| `DELETE` | `/entries/:id` | Delete a specific record |

## Project Structure

```
├── auth_service/          # User management microservice
├── training_service/      # Training generation microservice
├── records_service/       # Records & achievements microservice
├── docker-compose.yml     # Blue-Green Docker deployment
├── .github/workflows/     # CI/CD pipeline
└── README.md
```

Each service directory contains its own `main.cpp`, controllers, domain logic, and `Dockerfile`.

## Docker & Blue-Green Deployment

The system is fully containerized and uses Docker Compose. We implement a **Blue-Green Deployment** strategy with Docker Compose profiles.

```bash
# Start the Blue environment (default)
docker-compose up -d

# Start the Green environment (runs on ports 9080, 9081, 9082)
docker-compose --profile green up -d
```

### CI/CD Pipeline
The GitHub Actions pipeline runs on every push to `main` and verifies:
1. Compilation of all 3 microservices
2. Unit tests execution
3. Docker image builds
