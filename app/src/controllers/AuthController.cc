#include "comfyui_plus_backend/controllers/AuthController.h"
#include <json/json.h>     // For Json::Value, Drogon uses jsoncpp
#include <drogon/utils/FunctionTraits.h> // For traits if needed, often for callback types
#include <drogon/HttpTypes.h> // For k400BadRequest etc.
#include <drogon/drogon.h>    // For LOG_DEBUG etc.

// Use the namespace aliases for brevity
namespace cupb_controllers = comfyui_plus_backend::app::controllers;
namespace cupb_services = comfyui_plus_backend::app::services;


cupb_controllers::AuthController::AuthController()
    : authService_(std::make_shared<cupb_services::AuthService>())
{
    LOG_DEBUG << "AuthController constructed";
}

// Registration Handler
drogon::async_func::Task<void> cupb_controllers::AuthController::handleRegister(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << "Handling /auth/register request";
    auto jsonBodyPtr = req->getJsonObject(); // Returns std::shared_ptr<Json::Value>

    if (!jsonBodyPtr)
    {
        Json::Value errorJson;
        errorJson["error"] = "Invalid JSON payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(errorJson);
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        callback(resp);
        co_return;
    }
    const auto& jsonBody = *jsonBodyPtr; // Dereference for easier access

    // Validate presence of required fields
    if (!jsonBody.isMember("username") || !jsonBody["username"].isString() ||
        !jsonBody.isMember("email") || !jsonBody["email"].isString() ||
        !jsonBody.isMember("password") || !jsonBody["password"].isString())
    {
        Json::Value errorJson;
        errorJson["error"] = "Missing or invalid fields: username, email, and password are required strings.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(errorJson);
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        callback(resp);
        co_return;
    }

    std::string username = jsonBody["username"].asString();
    std::string email = jsonBody["email"].asString();
    std::string password = jsonBody["password"].asString();

    // Call the AuthService
    // If AuthService::registerUser were async (returning Task), you'd use co_await here.
    // For now, assuming it's synchronous as per our previous definition.
    auto [userOpt, errorMsg] = authService_->registerUser(username, email, password);

    if (userOpt)
    {
        Json::Value successJson;
        successJson["message"] = "User registered successfully.";
        // Construct a safe user object for the response (no sensitive data)
        Json::Value userJson;
        userJson["id"] = static_cast<Json::Int64>(userOpt->getId().value_or(0)); // Assuming getId() might be optional
        userJson["username"] = userOpt->getUsername();
        userJson["email"] = userOpt->getEmail();
        // Add other safe fields if your models::User DTO has them (e.g., createdAt)
        successJson["user"] = userJson;

        auto resp = drogon::HttpResponse::newHttpJsonResponse(successJson);
        resp->setStatusCode(drogon::HttpStatusCode::k201Created);
        callback(resp);
    }
    else
    {
        Json::Value errorJson;
        errorJson["error"] = errorMsg;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(errorJson);
        // Determine status code based on error type
        if (errorMsg.find("exists") != std::string::npos) {
            resp->setStatusCode(drogon::HttpStatusCode::k409Conflict);
        } else if (errorMsg.find("character") != std::string::npos || errorMsg.find("empty") != std::string::npos) {
            resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        }
        else {
            resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError); // Generic server error
        }
        callback(resp);
    }
    co_return; // Required for Task-based handlers
}

// Login Handler
drogon::async_func::Task<void> cupb_controllers::AuthController::handleLogin(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << "Handling /auth/login request";
    auto jsonBodyPtr = req->getJsonObject();

    if (!jsonBodyPtr)
    {
        Json::Value errorJson;
        errorJson["error"] = "Invalid JSON payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(errorJson);
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        callback(resp);
        co_return;
    }
    const auto& jsonBody = *jsonBodyPtr;

    if ((!jsonBody.isMember("emailOrUsername") || !jsonBody["emailOrUsername"].isString()) &&
        (!jsonBody.isMember("username") || !jsonBody["username"].isString()) &&
        (!jsonBody.isMember("email") || !jsonBody["email"].isString()) ||
        !jsonBody.isMember("password") || !jsonBody["password"].isString())
    {
        Json::Value errorJson;
        errorJson["error"] = "Missing or invalid fields: (emailOrUsername or email or username) and password are required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(errorJson);
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        callback(resp);
        co_return;
    }

    std::string loginIdentifier;
    if (jsonBody.isMember("emailOrUsername") && jsonBody["emailOrUsername"].isString()) {
        loginIdentifier = jsonBody["emailOrUsername"].asString();
    } else if (jsonBody.isMember("email") && jsonBody["email"].isString()) {
        loginIdentifier = jsonBody["email"].asString();
    } else if (jsonBody.isMember("username") && jsonBody["username"].isString()) {
        loginIdentifier = jsonBody["username"].asString();
    }

    std::string password = jsonBody["password"].asString();

    // Call the AuthService
    auto [tokenOpt, errorMsg] = authService_->loginUser(loginIdentifier, password);

    if (tokenOpt)
    {
        Json::Value successJson;
        successJson["message"] = "Login successful.";
        successJson["token"] = tokenOpt.value();
        // Optionally include some basic user info (non-sensitive)
        // You might fetch user details again based on token, or pass them from AuthService
        auto resp = drogon::HttpResponse::newHttpJsonResponse(successJson);
        callback(resp);
    }
    else
    {
        Json::Value errorJson;
        errorJson["error"] = errorMsg; // Should be "Invalid credentials." for security
        auto resp = drogon::HttpResponse::newHttpJsonResponse(errorJson);
        resp->setStatusCode(drogon::HttpStatusCode::k401Unauthorized);
        callback(resp);
    }
    co_return; // Required for Task-based handlers
}

// Implementation for getCurrentUser (example, will need JwtAuthFilter)
/*
drogon::async_func::Task<void> cupb_controllers::AuthController::getCurrentUser(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    // This assumes JwtAuthFilter has run and put user info into req attributes
    // or that we can get it from the token (if filter only validates)
    // For example, if filter sets "user_id" attribute:
    // if (req->attributes()->find("user_id") != req->attributes()->end()) {
    //     auto userId = req->attributes()->get<int64_t>("user_id");
    //     // ... fetch user details from UserService using userId ...
    //     // ... construct safe JSON response ...
    //     Json::Value userJson;
    //     userJson["message"] = "User data retrieved";
    //     userJson["user"]["id"] = userId;
    //     // userJson["user"]["username"] = ...
    //     auto resp = drogon::HttpResponse::newHttpJsonResponse(userJson);
    //     callback(resp);
    // } else {
    //     Json::Value errorJson;
    //     errorJson["error"] = "Unauthorized or user data not found in request.";
    //     auto resp = drogon::HttpResponse::newHttpJsonResponse(errorJson);
    //     resp->setStatusCode(drogon::HttpStatusCode::k401Unauthorized);
    //     callback(resp);
    // }
    // co_return;

    // For now, a placeholder:
    Json::Value respJson;
    respJson["message"] = "/auth/me endpoint hit (needs JWT filter and implementation)";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(respJson);
    callback(resp);
    co_return;
}
*/

} // namespace controllers
} // namespace app
} // namespace comfyui_plus_backend