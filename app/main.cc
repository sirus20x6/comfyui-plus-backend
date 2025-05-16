// app/main.cc
#include <drogon/drogon.h>
#include "comfyui_plus_backend/db/DatabaseManager.h"
// No need to include the filter header if not registering it manually

int main() {
    // Load config file
    drogon::app().loadConfigFile("config.json");
    
    // Set SQLite thread safety mode
    drogon::app().setThreadNum(1);
    
    // Initialize database
    auto& dbManager = comfyui_plus_backend::app::db::DatabaseManager::getInstance();
    dbManager.initialize("comfyui_plus.sqlite");
    
    // REMOVED: No manual filter registration needed
    
    // Log startup information
    LOG_INFO << "Server starting...";
    
    // Run the HTTP server
    drogon::app().run();
    
    return 0;
}