#pragma once

#include "comfyui_plus_backend/services/UserService.h"  // To interact with user data
#include "comfyui_plus_backend/services/JwtService.h"   // To generate JWTs
#include "comfyui_plus_backend/models/User.h"          // For returning user info (optional)
#include <string>
#include <optional>
#include <expected>  // C++23 feature for error handling
#include <utility>   // For std::pair
#include <memory>    // For std::make_shared

namespace comfyui_plus_backend
{
namespace app
{
namespace services
{

/**
 * @brief Service for handling authentication operations
 * 
 * This service provides methods for user registration and login,
 * coordinating between UserService for database operations and
 * JwtService for token generation.
 */
class AuthService
{
  public:
    /**
     * @brief Constructor that initializes required services
     */
    AuthService();

    /**
     * @brief Error type for authentication operations
     */
    struct AuthError {
        std::string message;
        int statusCode;  // HTTP status code to return

        // Constructor
        AuthError(std::string msg, int code = 400) 
            : message(std::move(msg)), statusCode(code) {}
    };

    /**
     * @brief Attempts to register a new user.
     * 
     * Uses C++23's std::expected for improved error handling.
     * 
     * @param username The desired username
     * @param email The user's email address
     * @param plainPassword The plain text password (will be hashed)
     * @return std::expected containing the created user or an AuthError
     */
    std::expected<comfyui_plus_backend::app::models::User, AuthError> registerUser(
        const std::string &username,
        const std::string &email,
        const std::string &plainPassword);

    /**
     * @brief Legacy method for registering a user (pair-based return)
     * 
     * @param username The desired username
     * @param email The user's email address
     * @param plainPassword The plain text password
     * @return A pair with the optional user and an error message
     */
    std::pair<std::optional<comfyui_plus_backend::app::models::User>, std::string> registerUserLegacy(
        const std::string &username,
        const std::string &email,
        const std::string &plainPassword);

    /**
     * @brief Attempts to log in a user.
     * 
     * Uses C++23's std::expected for improved error handling.
     * 
     * @param emailOrUsername Either the email or username to log in with
     * @param plainPassword The plain text password to verify
     * @return std::expected containing the JWT token or an AuthError
     */
    std::expected<std::string, AuthError> loginUser(
        const std::string &emailOrUsername,
        const std::string &plainPassword);

    /**
     * @brief Legacy method for logging in a user (pair-based return)
     * 
     * @param emailOrUsername Either the email or username
     * @param plainPassword The plain text password
     * @return A pair with the optional JWT token and an error message
     */
    std::pair<std::optional<std::string>, std::string> loginUserLegacy(
        const std::string &emailOrUsername,
        const std::string &plainPassword);

  private:
    /**
     * @brief User service for database operations
     */
    std::shared_ptr<UserService> userService_;
    
    /**
     * @brief JWT service for token operations
     */
    std::shared_ptr<JwtService> jwtService_;
};

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend