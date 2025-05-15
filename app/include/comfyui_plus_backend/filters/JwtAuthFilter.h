#pragma once

#include <drogon/HttpFilter.h>

namespace comfyui_plus_backend
{
namespace app
{
namespace filters
{

class JwtAuthFilter : public drogon::HttpFilter<JwtAuthFilter>
{
public:
    virtual void doFilter(const drogon::HttpRequestPtr& req,
                          drogon::FilterCallback&& fcb,
                          drogon::FilterChainCallback&& fccb) override;
};

} // namespace filters
} // namespace app
} // namespace comfyui_plus_backend