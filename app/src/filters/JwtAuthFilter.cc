#include "comfyui_plus_backend/filters/JwtAuthFilter.h"
#include "comfyui_plus_backend/services/JwtService.h"
#include <drogon/drogon.h>

using namespace comfyui_plus_backend::app::filters;
using namespace comfyui_plus_backend::app::services;

void JwtAuthFilter::doFilter(const drogon::HttpRequestPtr &req,
                          drogon::FilterCallback &&fcb,
                          drogon::FilterChainCallback &&fccb)
{
    LOG_DEBUG << "JwtAuthFilter processing request";
    
    // Initialize JWT service
    JwtService jwtService;
    
    // Extract token from Authorization header
    std::string authHeader = req->getHeader("Authorization");
    std::string token;
    
    // Check if Authorization header exists and starts with "Bearer "
    if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
        LOG_WARN << "Missing or invalid Authorization header";
        auto resp = drogon::HttpResponse::newHttpJsonResponse({{"error", "Unauthorized: Missing or invalid token"}});
        resp->setStatusCode(drogon::HttpStatusCode::k401Unauthorized);
        fcb(resp);
        return;
    }
    
    // Extract token (remove "Bearer " prefix)
    token = authHeader.substr(7);
    
    // Verify token
    auto decodedToken = jwtService.verifyToken(token);
    if (!decodedToken) {
        LOG_WARN << "Invalid JWT token";
        auto resp = drogon::HttpResponse::newHttpJsonResponse({{"error", "Unauthorized: Invalid token"}});
        resp->setStatusCode(drogon::HttpStatusCode::k401Unauthorized);
        fcb(resp);
        return;
    }
    
    // Extract user ID from token
    auto userId = jwtService.getUserIdFromToken(*decodedToken);
    if (!userId) {
        LOG_WARN << "JWT token valid but user_id claim not found";
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
    LOG_DEBUG << "JWT authentication successful for user ID: " << *userId;
    fccb();
}