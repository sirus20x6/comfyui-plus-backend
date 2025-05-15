#pragma once

#include <string>
#include <optional>
#include <chrono> // For std::chrono::system_clock
#include <jwt-cpp/jwt.h> // Main jwt-cpp header

namespace comfyui_plus_backend
{
namespace app
{
namespace services
{

class JwtService
{
  public:
    JwtService(); // Constructor to load configuration

    // Generates a JWT for a given user ID and username.
    // Returns the token string, or an empty string on failure.
    std::string generateToken(int64_t userId, const std::string &username);

    // Verifies a JWT string.
    // Returns a decoded JWT object on success, or std::nullopt on failure
    // (e.g., invalid signature, expired, malformed).
    std::optional<jwt::decoded_jwt<jwt::traits::kazuho_picojson>> verifyToken(const std::string &tokenString);

    // Extracts the user ID from a decoded token.
    // Returns std::nullopt if "user_id" claim is not present or not an integer.
    std::optional<int64_t> getUserIdFromToken(const jwt::decoded_jwt<jwt::traits::kazuho_picojson>& decodedToken);


  private:
    std::string jwtSecret_;
    std::string jwtIssuer_;
    std::string jwtAudience_; // Optional
    long jwtExpiresInSeconds_;

    // jwt-cpp verifier instance, configured once
    jwt::verifier<jwt::default_clock, jwt::traits::kazuho_picojson> verifier_;
};

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend