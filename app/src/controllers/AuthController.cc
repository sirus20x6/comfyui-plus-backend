#include "comfyui_plus_backend/controllers/AuthController.h"
#include <json/json.h>     // For Json::Value, Drogon uses jsoncpp
#include <drogon/utils/FunctionTraits.h> // For traits if needed, often for callback types
#include <drogon/HttpTypes.h> // For k400BadRequest etc.
#include <drogon/drogon.h>    // For LOG_DEBUG etc.

namespace cupb_controllers = comfyui_plus_backend::app::controllers;
namespace cupb_services = comfyui_plus_backend::app::services;


cupb_controllers::AuthController::AuthController()
    : authService_(std::make_shared<cupb_services::AuthService>())
{
    LOG_DEBUG << "AuthController constructed";
}

// Registration Handler
void cupb_controllers::AuthController::handleRegister(
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
        return;
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
        return;
    }

    std::string username = jsonBody["username"].asString();
    std::string email = jsonBody["email"].asString();
    std::string password = jsonBody["password"].asString();

    // Call the AuthService using the legacy return type for now
    auto [userOpt, errorMsg] = authService_->registerUserLegacy(username, email, password);

    if (userOpt)
    {
        Json::Value successJson;
        successJson["message"] = "User registered successfully.";
        // Construct a safe user object for the response (no sensitive data)
        Json::Value userJson;
        userJson["id"] = static_cast<Json::Int64>(userOpt->getId().value_or(0));
        userJson["username"] = userOpt->getUsername();
        userJson["email"] = userOpt->getEmail();
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
        // Try to determine status code from message
        if (errorMsg.find("exists") != std::string::npos) {
            resp->setStatusCode(drogon::HttpStatusCode::k409Conflict);
        } else if (errorMsg.find("character") != std::string::npos || errorMsg.find("empty") != std::string::npos) {
            resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        } else {
            resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        }
        callback(resp);
    }
}

// Login Handler
void cupb_controllers::AuthController::handleLogin(
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
        return;
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
        return;
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

    // Call the AuthService with the legacy return type
    auto [tokenOpt, errorMsg] = authService_->loginUserLegacy(loginIdentifier, password);

    if (tokenOpt)
    {
        Json::Value successJson;
        successJson["message"] = "Login successful.";
        successJson["token"] = *tokenOpt;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(successJson);
        callback(resp);
    }
    else
    {
        Json::Value errorJson;
        errorJson["error"] = errorMsg;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(errorJson);
        if (errorMsg.find("Invalid credentials") != std::string::npos) {
            resp->setStatusCode(drogon::HttpStatusCode::k401Unauthorized);
        } else {
            resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        }
        callback(resp);
    }
}