#pragma once

#include <sqlpp23/sqlpp23.h>  
#include "comfyui_plus_backend/models/User.h" // Your Drogon-style User model (DTO)
#include <sqlpp23/sqlite3/sqlite3.h>          // For sqlpp23 sqlite3 connection
#include <sqlpp23/sqlite3/database/connection_pool.h>  // For connection pool
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
    UserService(); // Constructor to initialize connection pool and DB config
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
    // Helper function to get a database connection from the pool
    std::shared_ptr<sqlpp::sqlite3::connection> getDbConnection();

    // Converts an sqlpp23 result row to your application's User model DTO
    // The RowType will depend on the columns selected in your query.
    template<typename RowType>
    comfyui_plus_backend::app::models::User rowToUserModel(const RowType& row);

    // Database configuration for sqlpp23, loaded from Drogon's app config
    sqlpp::sqlite3::connection_config dbConfig_;
    
    // Connection pool for better performance and thread safety
    std::shared_ptr<sqlpp::sqlite3::connection_pool> connectionPool_;
    
    // Initial pool size - can be adjusted based on expected load
    static constexpr size_t INITIAL_POOL_SIZE = 5;
};

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend