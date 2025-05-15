#pragma once // Ensure this is at the top

#include <drogon/HttpController.h>
#include "comfyui_plus_backend/services/AuthService.h" // Include your AuthService
#include <memory> // For std::make_shared

// Use an alias for the namespace to shorten later uses if you like
namespace cupb_models = comfyui_plus_backend::app::models;
namespace cupb_services = comfyui_plus_backend::app::services;

// It's good practice to put your controllers in a namespace
namespace comfyui_plus_backend
{
namespace app
{
namespace controllers
{

// Using 'final' can sometimes offer minor optimization hints to the compiler
// and clearly indicates it's not meant to be inherited from.
class AuthController final : public drogon::HttpController<AuthController>
{
  public:
    AuthController(); // Constructor to initialize services

    METHOD_LIST_BEGIN
    // Define your routes and the methods they map to.
    // The last argument is a list of HTTP methods allowed for this path.
    ADD_METHOD_TO(AuthController::handleRegister, "/auth/register", {drogon::HttpMethod::Post});
    ADD_METHOD_TO(AuthController::handleLogin, "/auth/login", {drogon::HttpMethod::Post});
    // Example for a protected route later:
    // ADD_METHOD_TO(AuthController::getCurrentUser, "/auth/me", {drogon::HttpMethod::Get}, "JwtAuthFilter");
    METHOD_LIST_END

    // Endpoint handler declarations
    // Using drogon::async_func::Task for asynchronous operations
    drogon::async_func::Task<void> handleRegister(const drogon::HttpRequestPtr &req,
                                                  std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    drogon::async_func::Task<void> handleLogin(const drogon::HttpRequestPtr &req,
                                               std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    // Example for a protected route later:
    // drogon::async_func::Task<void> getCurrentUser(const drogon::HttpRequestPtr &req,
    //                                               std::function<void(const drogon::HttpResponsePtr &)> &&callback);

  private:
    std::shared_ptr<cupb_services::AuthService> authService_;
};

} // namespace controllers
} // namespace app
} // namespace comfyui_plus_backend