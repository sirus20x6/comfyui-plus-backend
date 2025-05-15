#pragma once

#include "comfyui_plus_backend/db/DatabaseManager.h"
#include "comfyui_plus_backend/models/User.h" // Your Drogon-style User model (DTO)
#include <string>
#include <optional>
#include <memory> // For std::shared_ptr

namespace comfyui_plus_backend
{
namespace app
{
namespace services
{

class UserService
{
  public:
    UserService(); // Constructor to initialize database connection
    ~UserService(); // Destructor to ensure clean shutdown

    // Creates a user and returns the created user model (DTO), or std::nullopt on failure.
    // The password provided here is the plain text password.
    std::optional<comfyui_plus_backend::app::models::User> createUser(
        const std::string &username,
        const std::string &email,
        const std::string &plainPassword);

    // These methods return the User DTO (safe for client)
    std::optional<comfyui_plus_backend::app::models::User> getUserByEmail(const std::string &email);
    std::optional<comfyui_plus_backend::app::models::User> getUserByUsername(const std::string &username);
    std::optional<comfyui_plus_backend::app::models::User> getUserById(int64_t userId);

    // Helper to check if username or email already exists
    bool userExists(const std::string& username, const std::string& email);

    // Internal method for AuthService to get the hashed password for verification.
    // This should not be part of the public API of UserService if possible,
    // or should return a very specific internal struct.
    // For now, keeping it simple for AuthService to call.
    std::optional<std::string> getHashedPasswordForLogin(const std::string& emailOrUsername);

  private:
    // Access to the database storage
    db::DatabaseManager& dbManager_;

    // Converts DB model to your application's User model DTO
    comfyui_plus_backend::app::models::User dbModelToUserModel(const db::models::User& dbUser);

    // Thread safety is handled by sqlite_orm's internal locks and 
    // by having separate connections per thread if needed
};

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend