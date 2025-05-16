// app/main.cc
#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>
#include "comfyui_plus_backend/db/DatabaseManager.h"
#include "comfyui_plus_backend/filters/JwtAuthFilter.h"
#include <fstream>  // For std::ofstream
#include <sqlite3.h>  // Works with both SQLite and libSQL
#include <memory>
#include <filesystem>  // For std::filesystem
#include <iostream>

// Global variable to store our JWT config
Json::Value globalJwtConfig;

int main() {
    // Get the absolute path to the working directory
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path configPath = currentPath / "config.json";
    
    std::cout << "Current directory: " << currentPath.string() << std::endl;
    std::cout << "Looking for config at: " << configPath.string() << std::endl;
    
    // Read the config file manually first
    Json::Value config;
    bool configLoaded = false;
    
    if (std::filesystem::exists(configPath)) {
        std::cout << "Config file found at: " << configPath.string() << std::endl;
        
        // Try to read the config file
        std::ifstream configFile(configPath);
        if (configFile.is_open()) {
            std::string configContents((std::istreambuf_iterator<char>(configFile)),
                                       std::istreambuf_iterator<char>());
            configFile.close();
            
            // Parse the JSON
            Json::CharReaderBuilder builder;
            std::string parseErrors;
            std::istringstream configStream(configContents);
            
            bool parsingSuccessful = Json::parseFromStream(builder, configStream, &config, &parseErrors);
            
            if (parsingSuccessful) {
                std::cout << "JSON parsing successful!" << std::endl;
                if (config.isMember("jwt")) {
                    std::cout << "JWT section found in config!" << std::endl;
                    // Store the JWT config for later use
                    globalJwtConfig = config["jwt"];
                    configLoaded = true;
                } else {
                    std::cout << "JWT section NOT found in manually parsed config!" << std::endl;
                }
            } else {
                std::cerr << "Error parsing JSON config: " << parseErrors << std::endl;
            }
        } else {
            std::cerr << "Failed to open config file for reading!" << std::endl;
        }
    }
    
    // If config wasn't loaded, create a default one
    if (!configLoaded) {
        std::cerr << "Config file not found or invalid, creating a new one..." << std::endl;
        
        // Initialize config with default values
        // Add listeners section
        Json::Value listeners(Json::arrayValue);
        Json::Value listener;
        listener["address"] = "0.0.0.0";
        listener["port"] = 8080;
        listener["https"] = false;
        listeners.append(listener);
        config["listeners"] = listeners;
        
        // Add DB clients section
        Json::Value dbClients(Json::arrayValue);
        Json::Value dbClient;
        dbClient["name"] = "default";
        dbClient["rdbms"] = "sqlite3";
        dbClient["dbname"] = "comfyui_plus.sqlite";
        dbClient["host"] = "localhost";
        dbClient["port"] = 5432;
        dbClient["user"] = "";
        dbClient["passwd"] = "";
        dbClient["is_fast"] = false;
        dbClient["connection_number"] = 1;
        dbClient["filename"] = "comfyui_plus.sqlite";
        dbClients.append(dbClient);
        config["db_clients"] = dbClients;
        
        // Add app section
        Json::Value app;
        app["number_of_threads"] = 1;
        app["log_path"] = "./";
        app["log_level"] = "DEBUG";
        config["app"] = app;
        
        // Add JWT section
        Json::Value jwt;
        jwt["secret"] = "your-secret-key-should-be-long-and-secure";
        jwt["issuer"] = "comfyui-plus";
        jwt["audience"] = "web-app";
        jwt["expires_in_seconds"] = 3600;
        config["jwt"] = jwt;
        
        // Store the JWT config for later use
        globalJwtConfig = jwt;
        
        // Write the config to a file
        std::ofstream configOutFile(configPath);
        if (configOutFile.is_open()) {
            configOutFile << config.toStyledString();
            configOutFile.close();
            std::cout << "Created config.json in: " << configPath.string() << std::endl;
        } else {
            std::cerr << "Failed to create config.json file!" << std::endl;
        }
    }
    
    // Initialize Drogon app with our config
    drogon::app().setLogLevel(trantor::Logger::kDebug);
    drogon::app().addListener("0.0.0.0", 8080);
    
    // Initialize database client manually if needed
    // We'll use the DatabaseManager's initialization instead
    
    // Log SQL information
    LOG_INFO << "Using SQL library: " << sqlite3_libversion();
    int threadsafe = sqlite3_threadsafe();
    LOG_INFO << "SQL thread safety level: " << threadsafe 
             << " (0=single-thread, 1=serialized, 2=multi-thread)";
    
    // Initialize database
    auto& dbManager = comfyui_plus_backend::app::db::DatabaseManager::getInstance();
    dbManager.initialize("comfyui_plus.sqlite");
    
    // Create a JWT filter instance
    auto jwtFilter = std::make_shared<comfyui_plus_backend::app::filters::JwtAuthFilter>();
    
    // Register the filter
    drogon::app().registerFilter(jwtFilter);
    
    // Log startup information
    LOG_INFO << "Server starting...";
    
    // Run the HTTP server
    drogon::app().run();
    
    return 0;
}