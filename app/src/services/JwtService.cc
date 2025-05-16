// app/src/services/JwtService.cc
#include "comfyui_plus_backend/services/JwtService.h"
#include <drogon/drogon.h> // For app().getCustomConfig() and LOG_ERROR
#include <memory>          // For std::make_unique if needed (not directly here)

// Add this at the top of the file, before namespace declarations
extern Json::Value globalJwtConfig;

namespace comfyui_plus_backend
{
namespace app
{
namespace services
{

JwtService::JwtService()
    : verifier_(jwt::verify<jwt::traits::kazuho_picojson>()) // Initialize with default verifier
{
    try
    {
        // Try to get the config directly from the app instance
        const auto &jsonConfig = drogon::app().getCustomConfig();
        
        // Debug the raw configuration to see what we're working with
        LOG_DEBUG << "Drogon config: " << (jsonConfig.isNull() ? "null" : jsonConfig.toStyledString());
        
        // Use our global config if Drogon's is null
        const Json::Value& jwtConfig = jsonConfig.isNull() || !jsonConfig.isMember("jwt") 
            ? globalJwtConfig 
            : jsonConfig["jwt"];
        
        // Debug the JWT section
        LOG_DEBUG << "JWT config being used: " << (jwtConfig.isNull() ? "null" : jwtConfig.toStyledString());
        
        if (jwtConfig.isNull()) {
            LOG_FATAL << "JWT configuration is null - neither from app config nor global config!";
            return;
        }
        
        if (jwtConfig.isMember("secret") && jwtConfig["secret"].isString()) {
            jwtSecret_ = jwtConfig["secret"].asString();
            LOG_DEBUG << "JWT secret loaded successfully";
        } else {
            LOG_FATAL << "JWT secret not found or not a string in config";
        }

        if (jwtConfig.isMember("issuer") && jwtConfig["issuer"].isString()) {
            jwtIssuer_ = jwtConfig["issuer"].asString();
            LOG_DEBUG << "JWT issuer loaded successfully";
        } else {
            LOG_FATAL << "JWT issuer not found or not a string in config";
        }

        if (jwtConfig.isMember("audience") && jwtConfig["audience"].isString()) {
            jwtAudience_ = jwtConfig["audience"].asString();
            LOG_DEBUG << "JWT audience loaded successfully";
        }
        // Audience is optional, so no fatal log if not present

        if (jwtConfig.isMember("expires_in_seconds") && jwtConfig["expires_in_seconds"].isInt()) {
            jwtExpiresInSeconds_ = jwtConfig["expires_in_seconds"].asInt64();
            LOG_DEBUG << "JWT expires_in_seconds loaded successfully: " << jwtExpiresInSeconds_;
        } else {
            LOG_FATAL << "JWT expires_in_seconds not found or not an integer in config";
        }

        if (jwtSecret_.empty() || jwtIssuer_.empty() || jwtExpiresInSeconds_ <= 0) {
             LOG_FATAL << "JWT configuration is invalid (empty secret/issuer or non-positive expiration).";
             // In a real app, you might throw here or ensure app doesn't start
        }

        // Configure the verifier
        // Note: kazuho_picojson is one of the available traits for JSON handling.
        // jwt-cpp also supports nlohmann_json if you prefer and link it.
        verifier_.allow_algorithm(jwt::algorithm::hs256{jwtSecret_})
                .with_issuer(jwtIssuer_);
        
        if (!jwtAudience_.empty()) {
            verifier_.with_audience(jwtAudience_);
        }
        
        LOG_INFO << "JWT service initialized successfully";
    }
    catch (const std::exception &e)
    {
        LOG_FATAL << "Exception during JwtService construction: " << e.what();
        // Rethrow or handle appropriately
    }
}

std::string JwtService::generateToken(int64_t userId, const std::string &username)
{
    if (jwtSecret_.empty()) {
        LOG_ERROR << "Cannot generate JWT: Secret key is not configured.";
        return "";
    }

    try
    {
        // Create a picojson object for user_id
        picojson::value userIdValue(static_cast<double>(userId));
        jwt::basic_claim<jwt::traits::kazuho_picojson> userIdClaim(userIdValue);
        
        // Create a picojson object for username
        picojson::value usernameValue(username);
        jwt::basic_claim<jwt::traits::kazuho_picojson> usernameClaim(usernameValue);
        
        auto token = jwt::create<jwt::traits::kazuho_picojson>()
                         .set_issuer(jwtIssuer_)
                         .set_subject(std::to_string(userId)) // Standard "sub" claim for user ID
                         .set_audience(jwtAudience_) // Optional
                         .set_issued_at(std::chrono::system_clock::now())
                         .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{jwtExpiresInSeconds_})
                         .set_payload_claim("user_id", userIdClaim) // Custom claim
                         .set_payload_claim("username", usernameClaim) // Custom claim
                         .sign(jwt::algorithm::hs256{jwtSecret_});
        return token;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "Error generating JWT: " << e.what();
        return "";
    }
}

std::optional<jwt::decoded_jwt<jwt::traits::kazuho_picojson>> JwtService::verifyToken(const std::string &tokenString)
{
    if (jwtSecret_.empty()) {
        LOG_ERROR << "Cannot verify JWT: Secret key is not configured.";
        return std::nullopt;
    }
    if (tokenString.empty()) {
        LOG_DEBUG << "Attempt to verify empty token string.";
        return std::nullopt;
    }

    try
    {
        auto decoded_token = jwt::decode<jwt::traits::kazuho_picojson>(tokenString);
        
        // Perform verification using the pre-configured verifier
        verifier_.verify(decoded_token); // Throws on failure

        return decoded_token;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "Error verifying JWT: " << e.what();
        return std::nullopt;
    }
}

std::optional<int64_t> JwtService::getUserIdFromToken(const jwt::decoded_jwt<jwt::traits::kazuho_picojson>& decodedToken)
{
    try {
        if (decodedToken.has_payload_claim("user_id")) {
            auto claim = decodedToken.get_payload_claim("user_id");
            // jwt-cpp claim values need to be converted to the expected type
            if (claim.get_type() == jwt::json::type::integer || 
                claim.get_type() == jwt::json::type::number) {
                 // For picojson, numbers are stored as double, so convert back to int64_t
                 return static_cast<int64_t>(claim.as_number());
            } else if (claim.get_type() == jwt::json::type::string) {
                // Sometimes user_id might be encoded as string in JWT from other systems
                try {
                    return std::stoll(claim.as_string()); // string to long long
                } catch (const std::exception& e) {
                    LOG_WARN << "user_id claim found as string but could not convert to integer: " 
                             << e.what();
                }
            } else {
                LOG_WARN << "user_id claim found but is not a number or string.";
            }
        } else if (decodedToken.has_subject()) { // Fallback to standard "sub" claim
             std::string sub = decodedToken.get_subject();
             try {
                 return std::stoll(sub); // string to long long
             } catch (const std::invalid_argument& ia) {
                 LOG_WARN << "Subject claim '" << sub << "' is not a valid integer for user_id.";
             } catch (const std::out_of_range& oor) {
                 LOG_WARN << "Subject claim '" << sub << "' is out of range for user_id.";
             }
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "Error extracting user_id from token: " << e.what();
    }
    return std::nullopt;
}

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend