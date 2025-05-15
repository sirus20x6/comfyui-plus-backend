#pragma once

#include "comfyui_plus_backend/services/UserService.h" // To interact with user data
#include "comfyui_plus_backend/services/JwtService.h"   // To generate JWTs
#include "comfyui_plus_backend/models/User.h"         // For returning user info (optional)
#include <string>
#include <optional>
#include <utility> // For std::pair
#include <memory>  // For std::make_shared

namespace comfyui_plus_backend
{
namespace app
{
namespace services
{

class AuthService
{
  public:
    AuthService(); // Constructor

    // Attempts to register a new user.
    // Returns a pair:
    // - std::optional<models::User>: The created user model on success (excluding sensitive info).
    // - std::string: An error message if registration failed.
    std::pair<std::optional<comfyui_plus_backend::app::models::User>, std::string> registerUser(
        const std::string &username,
        const std::string &email,
        const std::string &plainPassword);

    // Attempts to log in a user.
    // Returns a pair:
    // - std::optional<std::string>: The JWT on successful login.
    // - std::string: An error message if login failed.
    std::pair<std::optional<std::string>, std::string> loginUser(
        const std::string &emailOrUsername,
        const std::string &plainPassword);

  private:
    std::shared_ptr<UserService> userService_;
    std::shared_ptr<JwtService> jwtService_;
    // PasswordUtils is static, so no member needed if all methods are static
};

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend