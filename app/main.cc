// comfyui-plus-backend/main.cc
#include <drogon/drogon.h>
// #include "comfyui_plus_backend/controllers/AuthController.h" // If manual registration needed
// #include "comfyui_plus_backend/filters/JwtAuthFilter.h"   // If manual registration needed

int main() {
    //Set HTTP listener address and port
    //drogon::app().addListener("0.0.0.0", 8080);

    //Load config file
    drogon::app().loadConfigFile("config.json");

    // Optionally, register controllers and filters programmatically if not using annotations
    // auto &app = drogon::app();
    // app.registerController(std::make_shared<comfyui_plus_backend::app::controllers::AuthController>());
    // app.registerFilter(std::make_shared<comfyui_plus_backend::app::filters::JwtAuthFilter>("JwtAuthFilter"));


    //Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}