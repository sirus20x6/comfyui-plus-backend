#include "comfyui_plus_backend/services/AuthService.h"
#include "comfyui_plus_backend/utils/PasswordUtils.h" // For password verification
#include <drogon/drogon.h> // For LOG_WARN, LOG_ERROR
#include <source_location> // For better error reporting

namespace comfyui_plus_backend
{
namespace app
{
namespace services
{

AuthService::AuthService()
    : userService_(std::make_shared<UserService>()),
      jwtService_(std::make_shared<JwtService>())
{
    LOG_DEBUG << "AuthService constructed";
}

std::expected<comfyui_plus_backend::app::models::User, AuthService::AuthError> 
AuthService::registerUser(
    const std::string &username,
    const std::string &email,
    const std::string &plainPassword)
{
    // Basic Validations (can be expanded)
    if (username.length() < 3) {
        return std::unexpected(AuthError("Username must be at least 3 characters long.", 400));
    }
    if (email.empty() || email.find('@') == std::string::npos) {
        return std::unexpected(AuthError("Please provide a valid email address.", 400));
    }
    if (plainPassword.length() < 8) {
        return std::unexpected(AuthError("Password must be at least 8 characters long.", 400));
    }

    // Check if user already exists (username or email)
    if (userService_->userExists(username, email)) {
        LOG_WARN << "Attempt to register existing username or email: " << username << "/" << email;
        return std::unexpected(AuthError("Username or email already exists.", 409)); // 409 Conflict
    }

    // Create user via UserService (which handles password hashing and transaction)
    auto createdUserOpt = userService_->createUser(username, email, plainPassword);

    if (!createdUserOpt) {
        auto loc = std::source_location::current();
        LOG_ERROR << "User registration failed at " << loc.file_name() << ":" << loc.line() 
                 << " for user: " << username;
        return std::unexpected(AuthError("Failed to register user. Please try again.", 500));
    }

    LOG_INFO << "User registered successfully: " << username;
    
    // Ensure the returned model doesn't contain sensitive data
    comfyui_plus_backend::app::models::User safeUserModel;
    safeUserModel.setId(createdUserOpt->getId().value_or(0));
    safeUserModel.setUsername(createdUserOpt->getUsername());
    safeUserModel.setEmail(createdUserOpt->getEmail());
    safeUserModel.setCreatedAt(createdUserOpt->getCreatedAt());
    safeUserModel.setUpdatedAt(createdUserOpt->getUpdatedAt());
    
    return safeUserModel;
}

// Legacy method implementation
std::pair<std::optional<comfyui_plus_backend::app::models::User>, std::string> 
AuthService::registerUserLegacy(
    const std::string &username,
    const std::string &email,
    const std::string &plainPassword)
{
    // Use the new method and convert the result
    auto result = registerUser(username, email, plainPassword);
    
    if (result.has_value()) {
        return {result.value(), "User registered successfully."};
    } else {
        return {std::nullopt, result.error().message};
    }
}

std::expected<std::string, AuthService::AuthError> 
AuthService::loginUser(
    const std::string &emailOrUsername,
    const std::string &plainPassword)
{
    if (emailOrUsername.empty() || plainPassword.empty()) {
        return std::unexpected(AuthError("Email/Username and password cannot be empty.", 400));
    }

    std::optional<comfyui_plus_backend::app::models::User> userOpt;

    // Try to find user by email first, then by username if email search fails
    if (emailOrUsername.find('@') != std::string::npos) {
        userOpt = userService_->getUserByEmail(emailOrUsername);
    }
    
    if (!userOpt) {
        userOpt = userService_->getUserByUsername(emailOrUsername);
    }

    if (!userOpt) {
        LOG_WARN << "Login attempt for non-existent user: " << emailOrUsername;
        return std::unexpected(AuthError("Invalid credentials.", 401)); // Generic message for security
    }

    // Get the stored hashed password
    std::optional<std::string> storedHashedPasswordOpt = 
        userService_->getHashedPasswordForLogin(emailOrUsername);
    
    if (!storedHashedPasswordOpt || storedHashedPasswordOpt->empty()) {
        auto loc = std::source_location::current();
        LOG_ERROR << "User " << emailOrUsername << " found but has no stored password hash at " 
                 << loc.file_name() << ":" << loc.line();
        return std::unexpected(AuthError("Login failed. Account issue.", 500));
    }

    std::string storedHashedPassword = *storedHashedPasswordOpt;

    if (utils::PasswordUtils::verifyPassword(plainPassword, storedHashedPassword)) {
        // Password matches, generate JWT
        int64_t userId = userOpt->getId().value_or(0);
        if (userId == 0) {
            auto loc = std::source_location::current();
            LOG_ERROR << "User " << emailOrUsername << " has invalid ID after login at "
                     << loc.file_name() << ":" << loc.line();
            return std::unexpected(AuthError("Login failed due to account data issue.", 500));
        }
        
        std::string token = jwtService_->generateToken(userId, userOpt->getUsername());
        if (token.empty()) {
            auto loc = std::source_location::current();
            LOG_ERROR << "Failed to generate JWT for user: " << userOpt->getUsername() 
                     << " at " << loc.file_name() << ":" << loc.line();
            return std::unexpected(AuthError("Login failed: Could not issue session token.", 500));
        }
        
        LOG_INFO << "User logged in successfully: " << userOpt->getUsername();
        return token;
    }
    else {
        LOG_WARN << "Failed login attempt for user: " << emailOrUsername;
        return std::unexpected(AuthError("Invalid credentials.", 401)); // Generic message
    }
}

// Legacy method implementation
std::pair<std::optional<std::string>, std::string> 
AuthService::loginUserLegacy(
    const std::string &emailOrUsername,
    const std::string &plainPassword)
{
    // Use the new method and convert the result
    auto result = loginUser(emailOrUsername, plainPassword);
    
    if (result.has_value()) {
        return {result.value(), "Login successful."};
    } else {
        return {std::nullopt, result.error().message};
    }
}

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend