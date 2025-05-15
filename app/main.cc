
// comfyui-plus-backend/main.cc
#include <drogon/drogon.h>
#include "comfyui_plus_backend/controllers/AuthController.h"
#include "comfyui_plus_backend/filters/JwtAuthFilter.h"

int main() {
    // Load config file
    drogon::app().loadConfigFile("config.json");

    // Register controllers and filters programmatically
    auto &app = drogon::app();
    app.registerController(std::make_shared<comfyui_plus_backend::app::controllers::AuthController>());
    app.registerFilter(std::make_shared<comfyui_plus_backend::app::filters::JwtAuthFilter>(), "JwtAuthFilter");

    // Run HTTP framework, the method will block in the internal event loop
    drogon::app().run();
    return 0;
}