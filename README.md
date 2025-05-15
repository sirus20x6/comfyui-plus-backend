# ComfyUI Plus - Backend

This repository contains the backend service for ComfyUI Plus, a platform designed to provide an enhanced, performant, and feature-rich experience for creating, sharing, and discovering AI image generation workflows, inspired by ComfyUI.

## Project Intent & Vision

ComfyUI Plus aims to be a comprehensive, self-hostable platform for the AI art community. While initially leveraging a connection to a standard ComfyUI backend for workflow execution, the long-term vision includes direct integration with cloud-based AI model inferencing services and a robust set of community features. The core focus is on providing a highly performant user interface for graph/node editing and a scalable backend infrastructure.

## Core Goals

1.  **High-Performance Node Editor:** Deliver a WebGL/PixiJS based frontend that can smoothly handle extremely large and complex workflows. (Frontend component)
2.  **Self-Hostable & Open Source:** Provide a complete stack that users can deploy on their own infrastructure.
3.  **Robust User Management:** Secure user registration, login, and profile management.
4.  **Workflow Management:** Allow users to create, save, version, and manage their AI workflows.
5.  **Community & Sharing:** Enable users to share workflows, discover creations from others, and interact. (Future Goal)
6.  **Scalable Execution:** Initially connect to a user's local ComfyUI backend, with future plans for direct cloud AI service integration for workflow execution.
7.  **Modular Backend:** A C++ backend designed for performance and extensibility.

## Tech Stack (Backend)

*   **Programming Language:** C++ (targeting C++23)
*   **Web Framework:** [Drogon](https://github.com/drogonframework/drogon) (High-performance C++ HTTP framework)
*   **Database:** SQLite (Initial phase, self-contained)
*   **Database Access/ORM:** [sqlite_orm](https://github.com/fnc12/sqlite_orm) (Header-only ORM for SQLite)
*   **Authentication:** JSON Web Tokens (JWT)
    *   **JWT Library:** [jwt-cpp](https://github.com/Thalhammer/jwt-cpp)
*   **Password Hashing:** Argon2 (via `libargon2`)
*   **Build System:** CMake (using a Superbuild pattern for dependencies)
*   **Object Storage (Future/Optional Self-Hosted):** MinIO (S3-compatible, for avatars, thumbnails, etc.)

## Current Progress & Todos

This section tracks our development journey.

### Phase 0: Project Setup & Core Dependencies
*   [x] Initial C++ Drogon project structure defined.
*   [x] CMake Superbuild setup for managing external dependencies:
    *   [x] jwt-cpp
    *   [x] sqlite_orm
    *   [x] Argon2 (system library)
    *   [x] SQLite3 (system library)
*   [ ] ~~Containerization (Docker)~~ (Deferred)

### Phase 1: User Authentication & Profile Basics
*   **Goal:** Users can register, log in, and basic user data is stored.
*   **Status:** In Progress
*   **Todos:**
    *   [x] `PasswordUtils` for hashing and verification (using Argon2).
    *   [x] `JwtService` for JWT generation and validation.
    *   [x] `DatabaseManager` for database connections and thread-safety.
    *   [x] Database models and schema setup with sqlite_orm.
    *   [x] `UserService` for user management:
        *   [x] User model creation
        *   [x] `createUser` method.
        *   [x] `getUserByEmail`, `getUserByUsername`, `getUserById` methods.
        *   [x] `getHashedPasswordForLogin` method.
        *   [x] `userExists` method.
    *   [x] `AuthService` to orchestrate login/registration.
    *   **Current Focus:** [ ] `AuthController` API endpoints (`/auth/register`, `/auth/login`).
    *   [ ] `JwtAuthFilter` for protecting routes.
    *   [ ] Basic User Profile API endpoint (`/users/me` or `/auth/me` - protected).
    *   [ ] API endpoint to update user profile information.

### Phase 2: Workflow Storage & Basic Management (Backend)
*   **Goal:** Store and retrieve workflow definitions (nodes, connections, metadata).
*   **Status:** Not Started
*   **Todos:**
    *   [x] Workflow database model and schema definition
    *   [ ] `WorkflowService`:
        *   [ ] `createWorkflow`
        *   [ ] `getWorkflowById`
        *   [ ] `updateWorkflow`
        *   [ ] `deleteWorkflow`
        *   [ ] `listWorkflowsByUser`
        *   [ ] `listPublicWorkflows`
    *   [ ] `WorkflowController` API endpoints.
    *   [ ] Thumbnail storage integration (initially filesystem, later MinIO).

### Phase 3: ComfyUI Backend Proxying (Initial Execution)
*   **Goal:** Allow the frontend (via this backend) to submit workflows to a user-configured ComfyUI instance.
*   **Status:** Not Started
*   **Todos:**
    *   [ ] API endpoint to configure user's ComfyUI backend URL.
    *   [ ] Service to proxy requests (`/prompt`, `/object_info`, etc.) to the ComfyUI backend.
    *   [ ] Handling WebSocket events from ComfyUI and relaying to the frontend if necessary.

### Phase 4: Advanced Features & Cloud Integration (Future)
*   **Status:** Not Started
*   **Todos:**
    *   [ ] Workflow versioning.
    *   [ ] Workflow sharing and discovery features.
    *   [ ] Community interaction (comments, likes).
    *   [ ] Direct cloud AI model inference integration.
    *   [ ] Object storage for workflow assets (MinIO).
    *   [ ] Admin panel.
    *   [ ] Containerization and deployment guides.

## Getting Started (Development)

1.  **Prerequisites:**
    *   C++ Compiler (supporting C++23, e.g., GCC 13+, Clang 16+)
    *   CMake (version 3.10 or newer recommended)
    *   Git
    *   `libsqlite3-dev` (SQLite3 development headers and library)
    *   `libargon2-0-dev` (Argon2 development headers and library)
    *   Drogon framework installed system-wide or its headers/libraries available in a known path for CMake.
        *   *(Alternatively, Drogon could be added as an ExternalProject, but this adds complexity).*

2.  **Clone the repository:**
    ```bash
    git clone <repository_url>
    cd comfyui-plus-backend
    ```

3.  **Configure and Build:**
    ```bash
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug .. # Or Release
    cmake --build .
    ```

4.  **Run the backend:**
    The executable will be located in `build/_external_projects/app_build/ComfyUIPlusBackend` (or similar, based on the `PROJECT_NAME` in `app/CMakeLists.txt`).
    ```bash
    ./_external_projects/app_build/ComfyUIPlusBackend
    ```
    Ensure `config.json` is present in the same directory as the executable (the build system should copy it).

## API Endpoints (Planned / In Progress)

*   `POST /auth/register` - Register a new user.
*   `POST /auth/login` - Log in an existing user, returns JWT.
*   `GET /auth/me` - (Protected) Get current user's profile.

## Database Structure

The application uses SQLite with the sqlite_orm library to manage the following tables:

1. **users**
   - id (PRIMARY KEY)
   - username (UNIQUE)
   - email (UNIQUE)
   - hashed_password
   - created_at
   - updated_at

2. **workflows**
   - id (PRIMARY KEY)
   - user_id (FOREIGN KEY → users.id)
   - name
   - description
   - json_data
   - thumbnail_path
   - created_at
   - updated_at
   - is_public

3. **tags**
   - id (PRIMARY KEY)
   - name (UNIQUE)

4. **workflow_tags** (junction table)
   - id (PRIMARY KEY)
   - workflow_id (FOREIGN KEY → workflows.id)
   - tag_id (FOREIGN KEY → tags.id)
   - UNIQUE constraint on (workflow_id, tag_id)

## Moving from sqlpp23 to sqlite_orm

Initially, this project used sqlpp23 for database access. We have since migrated to sqlite_orm, which offers:

1. **Simplified Integration**: Header-only library with no code generation required
2. **Modern C++ Interface**: Clean API with strong typing
3. **Thread Safety**: Connection per thread model for SQLite concurrency
4. **Seamless Transactions**: Easy transaction management

## Contribution

Details on how to contribute will be added soon. For now, feel free to open issues or submit pull requests.

---