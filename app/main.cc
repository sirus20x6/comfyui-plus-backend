// app/main.cc
#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>
#include "comfyui_plus_backend/db/DatabaseManager.h"
#include "comfyui_plus_backend/filters/JwtAuthFilter.h"
#include <sqlite3.h>  // Works with both SQLite and libSQL
#include <memory>

int main() {
    // Load config file first so other components can access it
    drogon::app().loadConfigFile("config.json");
    
    // Check SQL library version
    LOG_INFO << "Using SQL library: " << sqlite3_libversion();
    
    // Check threading support
    int threadsafe = sqlite3_threadsafe();
    LOG_INFO << "SQL thread safety level: " << threadsafe 
             << " (0=single-thread, 1=serialized, 2=multi-thread)";

    // Since we're having issues with direct DbConfig creation, let's use 
    // the loadConfigFile approach which already sets up the database
    // The database is already configured in config.json
    
    // Initialize database
    auto& dbManager = comfyui_plus_backend::app::db::DatabaseManager::getInstance();
    dbManager.initialize("comfyui_plus.sqlite");
    
    // Create a JWT filter instance
    auto jwtFilter = std::make_shared<comfyui_plus_backend::app::filters::JwtAuthFilter>();
    
    // Register the filter - note that we can't register by path
    // We'll have to check paths inside the filter itself
    drogon::app().registerFilter(jwtFilter);
    
    // Log startup information
    LOG_INFO << "Server starting...";
    
    // Run the HTTP server
    drogon::app().run();
    
    return 0;
}