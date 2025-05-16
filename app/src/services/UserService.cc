#include "comfyui_plus_backend/services/UserService.h"
#include "comfyui_plus_backend/utils/PasswordUtils.h"
#include <drogon/drogon.h>
#include <chrono>

namespace comfyui_plus_backend
{
namespace app
{
namespace services
{

UserService::UserService()
    : dbManager_(db::DatabaseManager::getInstance())
{
    LOG_DEBUG << "UserService constructed";
}

UserService::~UserService()
{
    LOG_DEBUG << "UserService destroyed";
}

std::optional<comfyui_plus_backend::app::models::User> UserService::createUser(
    const std::string &username,
    const std::string &email,
    const std::string &plainPassword)
{
    if (!dbManager_.isInitialized()) {
        LOG_ERROR << "CreateUser: Database not initialized";
        return std::nullopt;
    }

    std::string hashedPassword = utils::PasswordUtils::hashPassword(plainPassword);
    if (hashedPassword.empty()) {
        LOG_ERROR << "CreateUser: Failed to hash password for user: " << username;
        return std::nullopt;
    }

    // Get the current time as an ISO string
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    char timeStr[100];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&nowTime));
    std::string timestamp(timeStr);

    try {
        auto& storage = dbManager_.getStorage();

        // Check if user already exists
        auto userExists = this->userExists(username, email);
        if (userExists) {
            LOG_WARN << "CreateUser: Username or email already exists: " << username << "/" << email;
            return std::nullopt;
        }

        // Create the database user model
        db::models::User dbUser;
        dbUser.username = username;
        dbUser.email = email;
        dbUser.hashedPassword = hashedPassword;
        dbUser.createdAt = timestamp;
        dbUser.updatedAt = timestamp;

        // Insert the user and get the ID
        auto insertedId = storage.insert(dbUser);
        
        // Set the ID in our user object
        dbUser.id = insertedId;
        
        // Convert to API model and return
        return dbModelToUserModel(dbUser);
    }
    catch (const std::exception &e) {
        LOG_ERROR << "Error creating user " << username << ": " << e.what();
        return std::nullopt;
    }
}

std::optional<comfyui_plus_backend::app::models::User> UserService::getUserByEmail(const std::string &email)
{
    if (!dbManager_.isInitialized()) {
        LOG_ERROR << "getUserByEmail: Database not initialized";
        return std::nullopt;
    }

    try {
        auto& storage = dbManager_.getStorage();
        
        // Query for user by email
        auto users = storage.get_all<db::models::User>(
            sqlite_orm::where(sqlite_orm::c(&db::models::User::email) == email),
            sqlite_orm::limit(1)
        );
        
        if (users.empty()) {
            return std::nullopt;
        }
        
        // Convert to API model and return
        return dbModelToUserModel(users.front());
    }
    catch (const std::exception &e) {
        LOG_ERROR << "Error getting user by email " << email << ": " << e.what();
        return std::nullopt;
    }
}

std::optional<comfyui_plus_backend::app::models::User> UserService::getUserByUsername(const std::string &username)
{
    if (!dbManager_.isInitialized()) {
        LOG_ERROR << "getUserByUsername: Database not initialized";
        return std::nullopt;
    }

    try {
        auto& storage = dbManager_.getStorage();
        
        // Query for user by username
        auto users = storage.get_all<db::models::User>(
            sqlite_orm::where(sqlite_orm::c(&db::models::User::username) == username),
            sqlite_orm::limit(1)
        );
        
        if (users.empty()) {
            return std::nullopt;
        }
        
        // Convert to API model and return
        return dbModelToUserModel(users.front());
    }
    catch (const std::exception &e) {
        LOG_ERROR << "Error getting user by username " << username << ": " << e.what();
        return std::nullopt;
    }
}

std::optional<comfyui_plus_backend::app::models::User> UserService::getUserById(int64_t userId)
{
    if (!dbManager_.isInitialized()) {
        LOG_ERROR << "getUserById: Database not initialized";
        return std::nullopt;
    }

    try {
        auto& storage = dbManager_.getStorage();
        
        // Query for user by ID
        auto userOpt = storage.get_optional<db::models::User>(userId);
        
        if (!userOpt.has_value()) {
            return std::nullopt;
        }
        
        // Convert to API model and return
        return dbModelToUserModel(userOpt.value());
    }
    catch (const std::exception &e) {
        LOG_ERROR << "Error getting user by ID " << userId << ": " << e.what();
        return std::nullopt;
    }
}

std::optional<std::string> UserService::getHashedPasswordForLogin(const std::string& emailOrUsername)
{
    if (!dbManager_.isInitialized()) {
        LOG_ERROR << "getHashedPasswordForLogin: Database not initialized";
        return std::nullopt;
    }
    
    try {
        auto& storage = dbManager_.getStorage();
        
        // First try by email
        auto usersByEmail = storage.get_all<db::models::User>(
            sqlite_orm::where(sqlite_orm::c(&db::models::User::email) == emailOrUsername),
            sqlite_orm::limit(1)
        );
        
        // If not found by email, try by username
        if (usersByEmail.empty()) {
            auto usersByUsername = storage.get_all<db::models::User>(
                sqlite_orm::where(sqlite_orm::c(&db::models::User::username) == emailOrUsername),
                sqlite_orm::limit(1)
            );
            
            if (usersByUsername.empty()) {
                return std::nullopt;
            }
            
            return usersByUsername.front().hashedPassword;
        }
        
        return usersByEmail.front().hashedPassword;
    }
    catch (const std::exception &e) {
        LOG_ERROR << "Error getting hashed password for " << emailOrUsername << ": " << e.what();
        return std::nullopt;
    }
}

bool UserService::userExists(const std::string& username, const std::string& email)
{
    if (!dbManager_.isInitialized()) {
        LOG_ERROR << "userExists: Database not initialized";
        return true; // Safer to return true if we can't check
    }

    try {
        auto& storage = dbManager_.getStorage();
        
        // Check if a user with the given username or email exists
        auto count = storage.count<db::models::User>(
            sqlite_orm::where(
                sqlite_orm::c(&db::models::User::username) == username
                or sqlite_orm::c(&db::models::User::email) == email
            )
        );
        
        return count > 0;
    }
    catch (const std::exception &e) {
        LOG_ERROR << "Error in userExists check: " << e.what();
        return true; // Safer to return true on error
    }
}

comfyui_plus_backend::app::models::User UserService::dbModelToUserModel(const db::models::User& dbUser)
{
    comfyui_plus_backend::app::models::User userModel;
    
    // Convert db model to API model
    if (dbUser.id.has_value()) {
        userModel.setId(dbUser.id.value());
    }
    
    userModel.setUsername(dbUser.username);
    userModel.setEmail(dbUser.email);
    
    // Don't copy the hashed password to the API model for security
    
    // Convert string timestamps to trantor::Date
    trantor::Date createdDate = trantor::Date::fromDbStringLocal(dbUser.createdAt);
    trantor::Date updatedDate = trantor::Date::fromDbStringLocal(dbUser.updatedAt);
    
    userModel.setCreatedAt(createdDate);
    userModel.setUpdatedAt(updatedDate);
    
    return userModel;
}

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend