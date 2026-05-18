# Training System — Microservices Architecture

Personalized Training System — A distributed C++ REST microservice architecture with a premium glassmorphic frontend, SQLite databases, IoC/DI, CI/CD pipelines, and secure Nginx reverse proxy routing.

## Architecture Overview

The system consists of three independent backend microservices and a unified frontend:

1. **Frontend (Nginx, Port 80/443)** — A premium HTML/CSS/JS glassmorphic single-page application. Handles routing, HTTPS (Let's Encrypt), and DDoS protection.
2. **Auth Service (Port 8082)** — Manages user registration, authentication, and user profiles.
3. **Training Service (Port 8080)** — Generates and tracks personalized training programs.
4. **Records Service (Port 8081)** — Tracks best exercise weights and training history.

Each backend microservice uses:
- **HTTP REST API** with JSON responses (`cpp-httplib` + `json.hpp`)
- **IoC Container** — generic template-based dependency injection
- **SQLite Database** — separate database file per microservice, protected against SQL Injection via **Prepared Statements**.

## Security & Reliability

- **Nginx Rate Limiting (DDoS Protection):** 
  - `auth-service` limited to 3 req/s per IP to prevent brute-force attacks and bot registration.
  - Core API endpoints limited to 10-15 req/s.
  - Configured `client_max_body_size` and timeouts to prevent Slowloris attacks.
- **SQL Injection Prevention:** All user inputs are sanitized using SQLite Prepared Statements (`sqlite3_prepare_v2` and `sqlite3_bind_text`).
- **Automated Backups:** A `backup.sh` bash script combined with `cron` automatically backs up the production `users.db` daily, archiving up to 7 days of backups to prevent data loss.

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
├── frontend/              # Nginx, HTML/CSS/JS frontend SPA
├── auth_service/          # User management microservice
├── training_service/      # Training generation microservice
├── records_service/       # Records & achievements microservice
├── docker-compose.yml     # Blue-Green Docker deployment
├── backup.sh              # Cron backup script
├── .github/workflows/     # CI/CD pipeline
└── README.md
```

## Docker Deployment

The system is fully containerized and uses Docker Compose. 

```bash
# Start the production environment (Nginx + Microservices)
docker-compose up -d --build

# Restart Nginx after config changes
docker-compose restart frontend-blue
```

### CI/CD Pipeline
The GitHub Actions pipeline runs on every push to `main` and verifies:
1. Compilation of all 3 microservices
2. Unit tests execution
3. Docker image builds
