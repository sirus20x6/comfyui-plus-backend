// app/include/comfyui_plus_backend/filters/JwtAuthFilter.h
#pragma once

#include <drogon/HttpFilter.h>

namespace comfyui_plus_backend
{
namespace app
{
namespace filters
{

// Set isAutoCreation to false to allow manual registration
class JwtAuthFilter : public drogon::HttpFilter<JwtAuthFilter, false>
{
public:
    JwtAuthFilter() {
        // Constructor
        LOG_DEBUG << "JwtAuthFilter instantiated";
    }
    
    virtual void doFilter(const drogon::HttpRequestPtr& req,
                          drogon::FilterCallback&& fcb,
                          drogon::FilterChainCallback&& fccb) override;
};

} // namespace filters
} // namespace app
} // namespace comfyui_plus_backend