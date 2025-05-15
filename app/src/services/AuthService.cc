#include "comfyui_plus_backend/services/AuthService.h"
#include "comfyui_plus_backend/utils/PasswordUtils.h" // For password verification
#include <drogon/drogon.h> // For LOG_WARN, LOG_ERROR

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
    // Initialization if needed
}

std::pair<std::optional<models::User>, std::string> AuthService::registerUser(
    const std::string &username,
    const std::string &email,
    const std::string &plainPassword)
{
    // Basic Validations (can be expanded)
    if (username.length() < 3) {
        return {std::nullopt, "Username must be at least 3 characters long."};
    }
    if (email.empty()) { // Add more robust email validation if needed
        return {std::nullopt, "Email cannot be empty."};
    }
    if (plainPassword.length() < 8) {
        return {std::nullopt, "Password must be at least 8 characters long."};
    }

    // Check if user already exists (username or email)
    if (userService_->userExists(username, email)) {
        LOG_WARN << "Attempt to register existing username or email: " << username << "/" << email;
        return {std::nullopt, "Username or email already exists."};
    }

    // Create user via UserService (which handles password hashing)
    std::optional<models::User> createdUser = userService_->createUser(username, email, plainPassword);

    if (createdUser)
    {
        LOG_INFO << "User registered successfully: " << username;
        // Ensure the returned model doesn't contain sensitive data like hashed_password
        // The models::User DTO should be designed for this.
        models::User safeUserModel;
        safeUserModel.setId(createdUser->getId().value_or(0)); // Assuming getId returns optional
        safeUserModel.setUsername(createdUser->getUsername());
        safeUserModel.setEmail(createdUser->getEmail());
        // Add other safe fields like createdAt, etc. if present in models::User
        return {safeUserModel, "User registered successfully."};
    }
    else
    {
        LOG_ERROR << "User registration failed for: " << username;
        return {std::nullopt, "Failed to register user. Please try again."};
    }
}

std::pair<std::optional<std::string>, std::string> AuthService::loginUser(
    const std::string &emailOrUsername,
    const std::string &plainPassword)
{
    if (emailOrUsername.empty() || plainPassword.empty()) {
        return {std::nullopt, "Email/Username and password cannot be empty."};
    }

    std::optional<models::User> userOpt;

    // Try to find user by email first, then by username if email search fails
    // or if emailOrUsername doesn't look like an email.
    // A more robust way is to have separate login fields or let UserService handle this logic.
    if (emailOrUsername.find('@') != std::string::npos) { // Simple check for email format
        userOpt = userService_->getUserByEmail(emailOrUsername);
    }
    
    if (!userOpt) { // If not found by email or wasn't an email format
        userOpt = userService_->getUserByUsername(emailOrUsername);
    }

    if (!userOpt)
    {
        LOG_WARN << "Login attempt for non-existent user: " << emailOrUsername;
        return {std::nullopt, "Invalid credentials."}; // Generic message for security
    }

    // User found, now get the stored hashed password.
    // IMPORTANT: The `models::User` from `UserService` should ideally fetch the
    // hashed password for this internal check, but not expose it externally.
    // We'll assume `userService_->getUserBy...` fetches a full user record internally,
    // including the hashed password, even if the DTO model `models::User` doesn't always carry it.
    // For this to work, the `models::User` class needs `getHashedPassword()`.
    // Or, UserService provides a method like `getHashedPasswordForUser(id)`.

    // Let's refine: UserService should provide a method that returns the full internal user details
    // or we assume the models::User returned by UserService can hold the hash temporarily for this service.
    // For this example, let's assume models::User CAN hold it for this service's use.
    // **This implies `models::User` has `getHashedPassword()` and `setHashedPassword()`**
    // **and `UserService::rowToUserModel` populates it.**

    std::string storedHashedPassword = userOpt->getHashedPassword(); // Needs getHashedPassword() in models::User

    if (storedHashedPassword.empty()) {
        LOG_ERROR << "User " << emailOrUsername << " found but has no stored password hash.";
        return {std::nullopt, "Login failed. Account issue."};
    }


    if (utils::PasswordUtils::verifyPassword(plainPassword, storedHashedPassword))
    {
        // Password matches, generate JWT
        int64_t userId = userOpt->getId().value_or(0); // Assuming getId() returns optional<int64_t>
        if (userId == 0) {
            LOG_ERROR << "User " << emailOrUsername << " has invalid ID after login.";
            return {std::nullopt, "Login failed due to account data issue."};
        }
        
        std::string token = jwtService_->generateToken(userId, userOpt->getUsername());
        if (token.empty()) {
            LOG_ERROR << "Failed to generate JWT for user: " << userOpt->getUsername();
            return {std::nullopt, "Login failed: Could not issue session token."};
        }
        LOG_INFO << "User logged in successfully: " << userOpt->getUsername();
        return {token, "Login successful."};
    }
    else
    {
        LOG_WARN << "Failed login attempt for user: " << emailOrUsername;
        return {std::nullopt, "Invalid credentials."}; // Generic message
    }
}

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend