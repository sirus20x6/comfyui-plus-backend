// app/src/filters/JwtAuthFilter.cc
#include "comfyui_plus_backend/filters/JwtAuthFilter.h"
#include "comfyui_plus_backend/services/JwtService.h"
#include <drogon/drogon.h>
#include <memory>

namespace comfyui_plus_backend
{
namespace app
{
namespace filters
{

void JwtAuthFilter::doFilter(const drogon::HttpRequestPtr& req,
                        drogon::FilterCallback&& fcb,
                        drogon::FilterChainCallback&& fccb)
{
    // Check if this is a path we want to protect
    std::string path = req->getPath();
    
    // Skip if the path is not one we want to protect
    if (!isProtectedPath(path)) {
        fccb();
        return;
    }
    
    LOG_DEBUG << "JwtAuthFilter processing request for path: " << path;
    
    // Extract token from Authorization header
    std::string authHeader = req->getHeader("Authorization");
    std::string token;
    
    // Check if Authorization header exists and starts with "Bearer "
    if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
        LOG_WARN << "Missing or invalid Authorization header for path: " << path;
        auto resp = drogon::HttpResponse::newHttpJsonResponse({{"error", "Unauthorized: Missing or invalid token"}});
        resp->setStatusCode(drogon::HttpStatusCode::k401Unauthorized);
        fcb(resp);
        return;
    }
    
    // Extract token (remove "Bearer " prefix)
    token = authHeader.substr(7);
    
    // Initialize JWT service
    services::JwtService jwtService;
    
    // Verify token
    auto decodedToken = jwtService.verifyToken(token);
    if (!decodedToken) {
        LOG_WARN << "Invalid JWT token for path: " << path;
        auto resp = drogon::HttpResponse::newHttpJsonResponse({{"error", "Unauthorized: Invalid token"}});
        resp->setStatusCode(drogon::HttpStatusCode::k401Unauthorized);
        fcb(resp);
        return;
    }
    
    // Extract user ID from token
    auto userId = jwtService.getUserIdFromToken(*decodedToken);
    if (!userId) {
        LOG_WARN << "JWT token valid but user_id claim not found for path: " << path;
        auto resp = drogon::HttpResponse::newHttpJsonResponse({{"error", "Unauthorized: Invalid token format"}});
        resp->setStatusCode(drogon::HttpStatusCode::k401Unauthorized);
        fcb(resp);
        return;
    }
    
    // Add user ID to request attributes for controllers to use
    req->attributes()->insert("user_id", *userId);
    
    // If there's a username claim, add it too
    if (decodedToken->has_payload_claim("username")) {
        auto usernameClaim = decodedToken->get_payload_claim("username");
        if (usernameClaim.get_type() == jwt::json::type::string) {
            req->attributes()->insert("username", usernameClaim.as_string());
        }
    }
    
    // Continue the filter chain
    LOG_DEBUG << "JWT authentication successful for user ID: " << *userId << " on path: " << path;
    fccb();
}

bool JwtAuthFilter::isProtectedPath(const std::string& path) {
    // Check if the path starts with "/workflows" or is "/auth/me"
    return path.find("/workflows") == 0 || path == "/auth/me";
}

} // namespace filters
} // namespace app
} // namespace comfyui_plus_backend