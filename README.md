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
*   **Database Access/ORM:** [sqlpp23](https://github.com/rbock/sqlpp23) (Type-safe SQL EDSL for C++)
*   **Authentication:** JSON Web Tokens (JWT)
    *   **JWT Library:** [jwt-cpp](https://github.com/Thalhammer/jwt-cpp)
*   **Password Hashing:** Argon2 (via `libargon2`)
*   **Date/Time Utilities:** [Hinnant's date library](https://github.com/HowardHinnant/date) (often a dependency of sqlpp23)
*   **Build System:** CMake (using a Superbuild pattern for dependencies)
*   **Object Storage (Future/Optional Self-Hosted):** MinIO (S3-compatible, for avatars, thumbnails, etc.)

## Current Progress & Todos

This section tracks our development journey.

### Phase 0: Project Setup & Core Dependencies
*   [x] Initial C++ Drogon project structure defined.
*   [x] CMake Superbuild setup for managing external dependencies:
    *   [x] Hinnant's date library
    *   [x] jwt-cpp
    *   [x] sqlpp23 (with SQLite3 connector support)
    *   [x] Argon2 (system library)
    *   [x] SQLite3 (system library)
*   [ ] ~~Containerization (Docker)~~ (Deferred)

### Phase 1: User Authentication & Profile Basics
*   **Goal:** Users can register, log in, and basic user data is stored.
*   **Status:** In Progress
*   **Todos:**
    *   [x] `PasswordUtils` for hashing and verification (using Argon2).
    *   [x] `JwtService` for JWT generation and validation.
    *   [x] `UserService` (sqlpp23):
        *   [x] User schema definition for sqlpp23 (`db_schema/Users.h`).
        *   [x] Database table creation for users.
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
    *   [ ] Workflow database schema definition (sqlpp23).
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
    *   CMake (version 3.16 or newer recommended)
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

## Contribution

Details on how to contribute will be added soon. For now, feel free to open issues or submit pull requests.

---