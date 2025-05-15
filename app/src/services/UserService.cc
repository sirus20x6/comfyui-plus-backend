#include "comfyui_plus_backend/services/UserService.h"
#include "comfyui_plus_backend/db_schema/Users.h"    // Your sqlpp23 table schema
#include "comfyui_plus_backend/utils/PasswordUtils.h"
#include "comfyui_plus_backend/utils/DateTimeUtils.h"
#include <sqlpp23/sqlpp23.h>                         // Main sqlpp23 header
#include <sqlpp23/core/query/statement.h>            // For sqlpp::statement
#include <sqlpp23/core/clause/select.h>
#include <sqlpp23/core/clause/insert.h>
#include <sqlpp23/core/database/transaction.h>       // For transactions
#include <drogon/drogon.h>
#include <chrono>

namespace comfyui_plus_backend
{
namespace app
{
namespace services
{

namespace {
    // Create a single instance of the table object
    db_schema::Users users_table{};
}

UserService::UserService()
{
    try {
        // Get database path from Drogon config
        auto drogonDbClient = drogon::app().getDbClient("default");
        const auto& connInfo = drogonDbClient->connectionInfo();

        dbConfig_.path_to_database = connInfo.dbname();
        dbConfig_.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        
        // Enable SQL tracing in debug mode
        if (trantor::Logger::logLevel() == trantor::Logger::LogLevel::kDebug) {
            dbConfig_.debug = sqlpp::debug_logger{
                {sqlpp::log_category::all},
                [](const std::string& msg) { LOG_DEBUG << "[sqlpp23] " << msg; }
            };
        }
        
        // Initialize the connection pool
        connectionPool_ = std::make_shared<sqlpp::sqlite3::connection_pool>(dbConfig_, INITIAL_POOL_SIZE);
        LOG_INFO << "UserService: Database connection pool initialized with " << INITIAL_POOL_SIZE << " connections";

    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to initialize UserService DB config: " << e.what();
    }
}

UserService::~UserService()
{
    // The connection pool will be automatically destroyed when the shared_ptr is destroyed.
    // This is included for clarity and potential future cleanup operations.
    LOG_INFO << "UserService: Shutting down DB connection pool";
}

std::shared_ptr<sqlpp::sqlite3::connection> UserService::getDbConnection()
{
    try {
        // Get a connection from the pool
        return connectionPool_->get();
    } catch (const sqlpp::exception& e) {
        LOG_ERROR << "Failed to get SQLite connection from pool (sqlpp error): " << e.what();
        return nullptr;
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to get SQLite connection from pool: " << e.what();
        return nullptr;
    }
}

template<typename RowType>
comfyui_plus_backend::app::models::User UserService::rowToUserModel(const RowType& row) {
    comfyui_plus_backend::app::models::User userModel;

    // Handle ID with optional checking
    if constexpr (requires { row.id; }) {
        if (row.id) {
            userModel.setId(*row.id);
        }
    }

    // Handle string fields - convert string_view to std::string explicitly
    if constexpr (requires { row.username; }) {
        userModel.setUsername(std::string(row.username.data(), row.username.size()));
    }
    
    if constexpr (requires { row.email; }) {
        userModel.setEmail(std::string(row.email.data(), row.email.size()));
    }
    
    // Handle date fields using the utility class
    if constexpr (requires { row.created_at; }) {
        userModel.setCreatedAt(utils::DateTimeUtils::nullableToDate(row.created_at));
    }
    
    if constexpr (requires { row.updated_at; }) {
        userModel.setUpdatedAt(utils::DateTimeUtils::nullableToDate(row.updated_at));
    }
    
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
        // Start a transaction for atomicity
        auto tx = sqlpp::start_transaction(*db);
        
        // Prepare a statement to check existence (using named parameters)
        auto userExistsStmt = db->prepare(
            sqlpp::select(sqlpp::count(users_table.id))
                .from(users_table)
                .where(users_table.username == parameter(users_table.username) or 
                       users_table.email == parameter(users_table.email))
        );
        
        // Set parameters and execute
        userExistsStmt.params.username = username;
        userExistsStmt.params.email = email;
        
        auto existsResult = (*db)(userExistsStmt);
        if (!existsResult.empty() && existsResult.front().count > 0) {
            LOG_WARN << "CreateUser: Username or email already exists: " << username << "/" << email;
            return std::nullopt;
        }

        // Prepare insert statement (using parameters for all values)
        auto insertStmt = db->prepare(
            sqlpp::insert_into(users_table).set(
                users_table.username = parameter(users_table.username),
                users_table.email = parameter(users_table.email),
                users_table.hashed_password = parameter(users_table.hashed_password),
                users_table.created_at = parameter(users_table.created_at),
                users_table.updated_at = parameter(users_table.updated_at)
            )
        );
        
        // Set parameters
        insertStmt.params.username = username;
        insertStmt.params.email = email;
        insertStmt.params.hashed_password = hashedPassword;
        insertStmt.params.created_at = now;
        insertStmt.params.updated_at = now;
        
        // Execute the insert
        (*db)(insertStmt);
        
        // Get the last insert ID using the documented method
        int64_t inserted_id = db->last_insert_id();
        
        if (inserted_id > 0) {
            // Commit the transaction
            tx.commit();
            
            // Create and return the user model
            comfyui_plus_backend::app::models::User createdUser;
            createdUser.setId(inserted_id);
            createdUser.setUsername(username);
            createdUser.setEmail(email);
            createdUser.setCreatedAt(trantor::Date(std::chrono::duration_cast<std::chrono::milliseconds>(
                                         now.time_since_epoch()).count()));
            createdUser.setUpdatedAt(trantor::Date(std::chrono::duration_cast<std::chrono::milliseconds>(
                                         now.time_since_epoch()).count()));
            return createdUser;
        } else {
            LOG_ERROR << "User insertion failed for " << username << ", returned ID: " << inserted_id;
            // No need to explicitly rollback - destructor will handle it
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
        // Prepare a parameterized statement
        auto stmt = db->prepare(
            sqlpp::select(
                users_table.id,
                users_table.username,
                users_table.email,
                users_table.created_at,
                users_table.updated_at)
                .from(users_table)
                .where(users_table.email == parameter(users_table.email))
                .limit(1u)
        );
        
        // Set parameter and execute
        stmt.params.email = email;
        auto result = (*db)(stmt);
        
        if (!result.empty()) {
            return rowToUserModel(result.front());
        }
    }
    catch (const sqlpp::exception &e)
    {
        LOG_ERROR << "SQL error getting user by email " << email << ": " << e.what();
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
        // Prepare a parameterized statement
        auto stmt = db->prepare(
            sqlpp::select(
                users_table.id,
                users_table.username,
                users_table.email,
                users_table.created_at,
                users_table.updated_at)
                .from(users_table)
                .where(users_table.username == parameter(users_table.username))
                .limit(1u)
        );
        
        // Set parameter and execute
        stmt.params.username = username;
        auto result = (*db)(stmt);
        
        if (!result.empty()) {
            return rowToUserModel(result.front());
        }
    }
    catch (const sqlpp::exception &e)
    {
        LOG_ERROR << "SQL error getting user by username " << username << ": " << e.what();
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
        // Prepare a parameterized statement
        auto stmt = db->prepare(
            sqlpp::select(
                users_table.id,
                users_table.username,
                users_table.email,
                users_table.created_at,
                users_table.updated_at)
                .from(users_table)
                .where(users_table.id == parameter(users_table.id))
                .limit(1u)
        );
        
        // Set parameter and execute
        stmt.params.id = userId;
        auto result = (*db)(stmt);
        
        if (!result.empty()) {
            return rowToUserModel(result.front());
        }
    }
    catch (const sqlpp::exception &e)
    {
        LOG_ERROR << "SQL error getting user by ID " << userId << ": " << e.what();
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
        // Prepare a parameterized statement using dynamic conditions
        // We'll use the same parameter twice for both email and username checks
        auto stmt = db->prepare(
            sqlpp::select(users_table.hashed_password)
                .from(users_table)
                .where(sqlpp::dynamic(true, users_table.email == parameter(users_table.email) or 
                                      users_table.username == parameter(users_table.username)))
                .limit(1u)
        );
        
        // Set parameters and execute
        stmt.params.email = emailOrUsername;
        stmt.params.username = emailOrUsername;
        
        auto result = (*db)(stmt);
        
        if (!result.empty()) {
            return std::string(result.front().hashed_password.data(), 
                               result.front().hashed_password.size());
        }
    } catch (const sqlpp::exception& e) {
        LOG_ERROR << "SQL error getting hashed password for " << emailOrUsername << ": " << e.what();
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
        return true; // Safer to return true if we can't check
    }

    try {
        // Prepare a parameterized statement
        auto stmt = db->prepare(
            sqlpp::select(sqlpp::count(users_table.id))
                .from(users_table)
                .where(users_table.username == parameter(users_table.username) or 
                       users_table.email == parameter(users_table.email))
        );
        
        // Set parameters and execute
        stmt.params.username = username;
        stmt.params.email = email;
        
        auto result = (*db)(stmt);
        
        if (!result.empty()) {
            return result.front().count > 0;
        }
    } catch (const sqlpp::exception& e) {
        LOG_ERROR << "SQL error in userExists check: " << e.what();
        return true; // Safer to return true on error
    } catch (const std::exception& e) {
        LOG_ERROR << "Error in userExists check: " << e.what();
        return true; // Safer to return true on error
    }
    return false;
}

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend