#include "comfyui_plus_backend/services/UserService.h"
#include "comfyui_plus_backend/db_schema/Users.h"    // Your sqlpp23 table schema
#include "comfyui_plus_backend/utils/PasswordUtils.h"
#include <sqlpp23/sqlpp23.h>                          // Main sqlpp23 header
#include <sqlpp23/core/query/statement.h>                      // For sqlpp::statement
//#include <sqlpp23/clause_mixins.h>                  // For .where(), .from() etc.
#include <sqlpp23/core/clause/select.h>
#include <sqlpp23/core/clause/insert.h>
// #include <sqlpp23/update.h> // If you need update
// #include <sqlpp23/delete.h> // If you need delete_from (was remove_from)
#include <drogon/drogon.h>
#include <chrono>

namespace comfyui_plus_backend
{
namespace app
{
namespace services
{

namespace {
    Schema::Users users_table{}; // sqlpp23 table instance
}

UserService::UserService()
{
    try {
        auto drogonDbConfig = drogon::app().getDbClient("default")->getDbConfig();
        dbConfig_.path_to_database = drogonDbConfig.dbname;
        dbConfig_.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        // sqlpp23 config for logging is different:
        // dbConfig_.debug = drogon::app().getLogLevel() == trantor::Logger::LogLevel::kDebug;
        if (drogon::app().getLogLevel() == trantor::Logger::LogLevel::kDebug) {
            dbConfig_.set_trace_flags(sqlpp::sqlite3::trace_statement | sqlpp::sqlite3::trace_profile);
            dbConfig_.set_trace_logger([](const std::string_view message) {
                LOG_DEBUG << "[sqlpp23] " << message;
            });
        }

    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to initialize UserService DB config: " << e.what();
    }
}

std::shared_ptr<sqlpp::sqlite3::connection> UserService::getDbConnection()
{
    try {
        // sqlpp23 connection constructor might be slightly different
        return std::make_shared<sqlpp::sqlite3::connection>(dbConfig_);
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to get SQLite connection: " << e.what();
        return nullptr;
    }
}

// Adjusted for sqlpp23 result row members being directly accessible by name
template<typename Row>
models::User UserService::rowToUserModel(const Row& row) {
    models::User userModel;

    // With sqlpp23, row members often directly match the C++ names from SQLPP_COLUMN
    // and nullable types are std::optional
    if (row.id) userModel.setId(*row.id); // Assuming id is std::optional<int64_t> if nullable
    else if constexpr (std::is_same_v<decltype(row.id), int64_t>) userModel.setId(row.id); // If not optional

    userModel.setUsername(std::string(row.username)); // string_view to string
    userModel.setEmail(std::string(row.email));       // string_view to string
    
    // IMPORTANT: Do not set hashed password in a DTO going to the client.
    // This model is now assumed to be the DTO. If you need the hash internally,
    // fetch it separately or use a different internal model structure.
    // For this example, we will assume models::User DTO does NOT have hashedPassword.
    // If UserService needs it for AuthService, it should fetch it specifically.

    if (row.created_at) {
        auto tp = *row.created_at; // sqlpp::time_point is std::chrono::system_clock::time_point
        userModel.setCreatedAt(trantor::Date(std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count()));
    }
    if (row.updated_at) {
        auto tp = *row.updated_at;
        userModel.setUpdatedAt(trantor::Date(std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count()));
    }
    return userModel;
}

std::optional<models::User> UserService::createUser(
    const std::string &username,
    const std::string &email,
    const std::string &plainPassword)
{
    auto db = getDbConnection();
    if (!db) {
        LOG_ERROR << "CreateUser: Failed to get DB connection.";
        return std::nullopt;
    }

    std::string hashedPassword = utils::PasswordUtils::hashPassword(plainPassword);
    if (hashedPassword.empty()) {
        LOG_ERROR << "CreateUser: Failed to hash password for user: " << username;
        return std::nullopt;
    }

    auto now = std::chrono::system_clock::now();

    try
    {
        // Check existence (sqlpp23 style)
        auto statement = sqlpp::statement<sqlpp::sqlite3::connection_t>(*db)
            << select(sqlpp::count(users_table.id)).from(users_table)
            << where(users_table.username == username or users_table.email == email);
        
        auto result_rows = execute(statement); // Simpler execute for non-select usually
        if (!result_rows.empty() && result_rows.front().count > 0) {
             LOG_WARN << "CreateUser: Username or email already exists: " << username << "/" << email;
            return std::nullopt;
        }

        // Insert (sqlpp23 style)
        auto insert_statement = sqlpp::statement<sqlpp::sqlite3::connection_t>(*db)
            << insert_into(users_table).set(
                   users_table.username = username,
                   users_table.email = email,
                   users_table.hashed_password = hashedPassword, // SQL column name
                   users_table.created_at = now,
                   users_table.updated_at = now);
        
        int64_t inserted_id = execute(insert_statement); // execute for insert returns last_insert_rowid

        if (inserted_id > 0) {
            models::User createdUser;
            createdUser.setId(inserted_id);
            createdUser.setUsername(username);
            createdUser.setEmail(email);
            createdUser.setCreatedAt(trantor::Date(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()));
            createdUser.setUpdatedAt(trantor::Date(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()));
            return createdUser;
        } else {
            LOG_ERROR << "User insertion failed for " << username << ", returned ID: " << inserted_id;
            return std::nullopt;
        }
    }
    catch (const sqlpp::exception &e)
    {
        LOG_ERROR << "SQL error creating user " << username << ": " << e.what();
        return std::nullopt;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "Error creating user " << username << ": " << e.what();
        return std::nullopt;
    }
}


// Helper to get full user data including hash (for internal AuthService use)
// This is one way to solve the "AuthService needs hash" problem.
// AuthService would call this, then verify password, then call a separate
// getUserBy... method that returns the DTO without the hash.
// OR, AuthService takes on password verification directly if UserService provides the hash.
// For simplicity in this example, we'll assume AuthService gets the hash via the model.
// So, the rowToUserModel must be adapted if the DTO `models::User` does not carry hash.
// Let's assume models::User DTO does NOT carry hash.
// We need a way for AuthService to get the hash.

// getUserByEmail (returns DTO safe for client)
std::optional<models::User> UserService::getUserByEmail(const std::string &email)
{
    auto db = getDbConnection();
    if (!db) return std::nullopt;

    try
    {
        // Select only fields needed for the DTO, excluding hashed_password
        auto statement = sqlpp::statement<sqlpp::sqlite3::connection_t>(*db)
            << select(users_table.id, users_table.username, users_table.email, users_table.created_at, users_table.updated_at)
            .from(users_table)
            .where(users_table.email == email)
            .limit(1u);
        
        for (const auto &row : execute(statement)) // execute for select returns iterable rows
        {
            return rowToUserModel(row); // rowToUserModel should be adapted for this selected set
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "Error getting user by email " << email << ": " << e.what();
    }
    return std::nullopt;
}

// getUserByUsername (returns DTO safe for client)
std::optional<models::User> UserService::getUserByUsername(const std::string &username)
{
    auto db = getDbConnection();
    if (!db) return std::nullopt;
    try
    {
        auto statement = sqlpp::statement<sqlpp::sqlite3::connection_t>(*db)
            << select(users_table.id, users_table.username, users_table.email, users_table.created_at, users_table.updated_at)
            .from(users_table)
            .where(users_table.username == username)
            .limit(1u);
        for (const auto &row : execute(statement))
        {
            return rowToUserModel(row);
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "Error getting user by username " << username << ": " << e.what();
    }
    return std::nullopt;
}

// getUserById (returns DTO safe for client)
std::optional<models::User> UserService::getUserById(int64_t userId)
{
    auto db = getDbConnection();
    if (!db) return std::nullopt;
    try
    {
        auto statement = sqlpp::statement<sqlpp::sqlite3::connection_t>(*db)
            << select(users_table.id, users_table.username, users_table.email, users_table.created_at, users_table.updated_at)
            .from(users_table)
            .where(users_table.id == userId)
            .limit(1u);
        for (const auto &row : execute(statement))
        {
            return rowToUserModel(row);
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "Error getting user by ID " << userId << ": " << e.what();
    }
    return std::nullopt;
}


// This function is needed by AuthService to verify password.
// It should NOT be exposed in UserService's public API if possible, or named carefully.
// For now, let's assume AuthService has a way to get this.
// A cleaner way is for AuthService to take username/email,
// fetch the hash itself using a specific private method here, verify, then fetch user DTO.
//
// For this iteration, AuthService.cc will need to call a method like this.
// We can add it to UserService.h or AuthService can have its own DB interaction for this.
// Let's add a method to UserService to retrieve the hash:

// In UserService.h add:
// std::optional<std::string> getHashedPasswordByEmailOrUsername(const std::string& emailOrUsername);

// In UserService.cc implement:
/*
std::optional<std::string> UserService::getHashedPasswordByEmailOrUsername(const std::string& emailOrUsername) {
    auto db = getDbConnection();
    if (!db) return std::nullopt;
    try {
        auto statement = sqlpp::statement<sqlpp::sqlite3::connection_t>(*db)
            << select(users_table.hashed_password)
            .from(users_table)
            .where(users_table.email == emailOrUsername or users_table.username == emailOrUsername)
            .limit(1u);
        for (const auto& row : execute(statement)) {
            return std::string(row.hashed_password); // string_view to string
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "Error getting hashed password for " << emailOrUsername << ": " << e.what();
    }
    return std::nullopt;
}
*/
// Then AuthService would use this. For now, I'll keep AuthService.cc assuming it got the hash from a User model.
// You'll need to decide how `AuthService` gets the `hashedPassword` for verification.
// If `models::User` DTO is strictly client-safe, `AuthService` will need a way to fetch the hash,
// e.g. `userService_->getInternalUserDetails(emailOrUsername)` which returns a struct with the hash,
// or `userService_->getHashedPassword(emailOrUsername)`.

bool UserService::userExists(const std::string& username, const std::string& email)
{
    auto db = getDbConnection();
    if (!db) {
        LOG_ERROR << "userExists: Failed to get DB connection.";
        return true;
    }

    try {
        auto statement = sqlpp::statement<sqlpp::sqlite3::connection_t>(*db)
            << select(sqlpp::count(users_table.id))
            .from(users_table)
            .where(users_table.username == username or users_table.email == email);
        
        auto result_rows = execute(statement);
        if (!result_rows.empty()) {
            return result_rows.front().count > 0;
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "Error in userExists check: " << e.what();
        return true;
    }
    return false;
}


} // namespace services
} // namespace app
} // namespace comfyui_plus_backend