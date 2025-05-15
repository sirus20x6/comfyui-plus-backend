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
#include "comfyui_plus_backend/utils/DateTimeUtils.h"

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
template<typename RowType>
comfyui_plus_backend::app::models::User UserService::rowToUserModel(const RowType& row) {
    comfyui_plus_backend::app::models::User userModel;

    // With sqlpp23, row members directly match the C++ names from SQLPP_COLUMN
    // and nullable types are std::optional
    if (row.id) userModel.setId(*row.id); // Assuming id is std::optional<int64_t> if nullable
    else if constexpr (std::is_same_v<decltype(row.id), int64_t>) userModel.setId(row.id); // If not optional

    userModel.setUsername(std::string(row.username)); // string_view to string
    userModel.setEmail(std::string(row.email));       // string_view to string
    
    // Use the utility class for date conversions
    userModel.setCreatedAt(utils::DateTimeUtils::nullableToDate(row.created_at));
    userModel.setUpdatedAt(utils::DateTimeUtils::nullableToDate(row.updated_at));
    
    return userModel;
}

std::optional<comfyui_plus_backend::app::models::User> UserService::createUser(
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
            comfyui_plus_backend::app::models::User createdUser;
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


// getUserByEmail (returns DTO safe for client)
std::optional<comfyui_plus_backend::app::models::User> UserService::getUserByEmail(const std::string &email)
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
std::optional<comfyui_plus_backend::app::models::User> UserService::getUserByUsername(const std::string &username)
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
std::optional<comfyui_plus_backend::app::models::User> UserService::getUserById(int64_t userId)
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

// Implementation of getHashedPasswordForLogin
std::optional<std::string> UserService::getHashedPasswordForLogin(const std::string& emailOrUsername)
{
    auto db = getDbConnection();
    if (!db) {
        LOG_ERROR << "getHashedPasswordForLogin: Failed to get DB connection.";
        return std::nullopt;
    }
    
    try {
        auto statement = sqlpp::statement<sqlpp::sqlite3::connection_t>(*db)
            << select(users_table.hashed_password)
            .from(users_table)
            .where(users_table.email == emailOrUsername or users_table.username == emailOrUsername)
            .limit(1u);
        
        for (const auto& row : execute(statement)) {
            return std::string(row.hashed_password); // Convert string_view to string
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "Error getting hashed password for " << emailOrUsername << ": " << e.what();
    }
    
    return std::nullopt;
}

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