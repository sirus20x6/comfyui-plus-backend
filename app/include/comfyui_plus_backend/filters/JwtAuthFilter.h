// app/include/comfyui_plus_backend/filters/JwtAuthFilter.h
#pragma once

#include <drogon/HttpFilter.h>
#include <string>
#include <regex>
#include "comfyui_plus_backend/services/JwtService.h"  // Include JwtService
#include <jwt-cpp/jwt.h>  // Include JWT-CPP for JWT types

namespace comfyui_plus_backend
{
namespace app
{
namespace filters
{

// Set isAutoCreation to false since we'll register it manually
class JwtAuthFilter : public drogon::HttpFilter<JwtAuthFilter, false>
{
public:
    JwtAuthFilter() {
        // Constructor
        LOG_DEBUG << "JwtAuthFilter instantiated";
    }
    
    // Only declare the method, implementation will be in the .cc file
    virtual void doFilter(const drogon::HttpRequestPtr& req,
                          drogon::FilterCallback&& fcb,
                          drogon::FilterChainCallback&& fccb) override;

private:
    // Helper method to check if a path should be protected
    bool isProtectedPath(const std::string& path);
};

} // namespace filters
} // namespace app
} // namespace comfyui_plus_backend