// app/main.cc
#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>
#include "comfyui_plus_backend/db/DatabaseManager.h"
#include "comfyui_plus_backend/filters/JwtAuthFilter.h"
#include <sqlite3.h>  // Works with both SQLite and libSQL
#include <memory>

int main() {
    // Check SQL library version
    LOG_INFO << "Using SQL library: " << sqlite3_libversion();
    
    // Check threading support
    int threadsafe = sqlite3_threadsafe();
    LOG_INFO << "SQL thread safety level: " << threadsafe 
             << " (0=single-thread, 1=serialized, 2=multi-thread)";

    // Create the SQLite connection using the createDbClient method with full parameters
    // Most of these parameters are not used by SQLite, but we need to provide them anyway
    int connectionNumber = (threadsafe == 2) ? 5 : 1;
    
    // Parameters: dbType, host, port, username, passwd, dbName, connectionNum, filename, ...
    drogon::app().createDbClient("sqlite3", // dbType
                                 "", // host 
                                 0,  // port
                                 "", // username
                                 "", // password
                                 "comfyui_plus.sqlite", // database name
                                 connectionNumber,      // connection number
                                 "comfyui_plus.sqlite", // filename
                                 "", // additional connection string
                                 false,  // sslUsed
                                 ""); // caPath
    
    // Load config file
    drogon::app().loadConfigFile("config.json");
    
    // Initialize database
    auto& dbManager = comfyui_plus_backend::app::db::DatabaseManager::getInstance();
    dbManager.initialize("comfyui_plus.sqlite");
    
    // Register the JWT auth filter explicitly without the path pattern
    auto jwtFilter = std::make_shared<comfyui_plus_backend::app::filters::JwtAuthFilter>();
    drogon::app().registerFilter(jwtFilter);
    
    // Log startup information
    LOG_INFO << "Server starting...";
    
    // Run the HTTP server
    drogon::app().run();
    
    return 0;
}