#pragma once

#include "comfyui_plus_backend/models/User.h" // Your Drogon-style User model (for data transfer)
#include <sqlpp11/sqlite3/sqlite3.h>          // For sqlpp::sqlite3::connection
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
    UserService(); // Constructor, could initialize db connection manager

    // Creates a user and returns the created user model, or std::nullopt on failure
    // The password provided here is the plain text password.
    std::optional<models::User> createUser(
        const std::string &username,
        const std::string &email,
        const std::string &plainPassword);

    std::optional<models::User> getUserByEmail(const std::string &email);
    std::optional<models::User> getUserByUsername(const std::string &username);
    std::optional<models::User> getUserById(int64_t userId);

    // Helper to check if username or email already exists
    bool userExists(const std::string& username, const std::string& email);


  private:
    // Helper function to get a database connection.
    // In a real app, this would be more sophisticated (e.g., connection pool).
    // This could also be a member variable initialized in the constructor.
    std::shared_ptr<sqlpp::sqlite3::connection> getDbConnection();

    // Converts an sqlpp11 row to your application's User model
    models::User rowToUserModel(const auto& row); // Using 'auto' for template-like behavior

    // Database configuration, loaded from Drogon's app config
    sqlpp::sqlite3::connection_config dbConfig_;
};

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend