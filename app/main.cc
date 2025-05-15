// comfyui-plus-backend/main.cc
#include <drogon/drogon.h>
#include "comfyui_plus_backend/controllers/AuthController.h"
#include "comfyui_plus_backend/controllers/WorkflowController.h"
#include "comfyui_plus_backend/filters/JwtAuthFilter.h"
#include "comfyui_plus_backend/services/DbConnectionPool.h"

int main() {
    // Load config file
    drogon::app().loadConfigFile("config.json");
    
    // Initialize the DB connection pool
    const auto& config = drogon::app().getCustomConfig();
    bool dbInitialized = false;
    
    try {
        // Get database configuration from Drogon's config
        auto dbClients = drogon::app().getDbClientNames();
        if (dbClients.empty()) {
            LOG_FATAL << "No database clients configured in config.json";
            return 1;
        }
        
        // Get the default database client configuration
        auto defaultDbClient = drogon::app().getDbClient("default");
        std::string dbPath = defaultDbClient->connectionInfo().host();
        
        // Determine debug mode based on the application's log level
        bool debugMode = (drogon::app().getLogLevel() == trantor::Logger::LogLevel::kDebug);
        
        // Get connection pool size from config or use default
        size_t poolSize = 5; // Default pool size
        if (config.isMember("database") && config["database"].isMember("pool_size") && 
            config["database"]["pool_size"].isInt()) {
            poolSize = config["database"]["pool_size"].asInt();
        }
        
        // Initialize the connection pool
        dbInitialized = comfyui_plus_backend::app::services::DbConnectionPool::getInstance()
                           .initSqlitePool(dbPath, poolSize, debugMode);
        
        if (!dbInitialized) {
            LOG_FATAL << "Failed to initialize database connection pool";
            return 1;
        }
        
        LOG_INFO << "Database connection pool initialized with size: " << poolSize;
    }
    catch (const std::exception& e) {
        LOG_FATAL << "Exception during database initialization: " << e.what();
        return 1;
    }
    
    // Register controllers and filters programmatically
    auto &app = drogon::app();
    app.registerController(std::make_shared<comfyui_plus_backend::app::controllers::AuthController>());
    
    // Only register the WorkflowController if it's implemented
    try {
        app.registerController(std::make_shared<comfyui_plus_backend::app::controllers::WorkflowController>());
        LOG_INFO << "WorkflowController registered";
    } catch (const std::exception& e) {
        LOG_WARN << "WorkflowController registration skipped: " << e.what();
    }
    
    app.registerFilter(std::make_shared<comfyui_plus_backend::app::filters::JwtAuthFilter>(), "JwtAuthFilter");
    
    LOG_INFO << "ComfyUI Plus Backend starting...";
    
    // Run HTTP framework, the method will block in the internal event loop
    drogon::app().run();
    
    LOG_INFO << "ComfyUI Plus Backend shutting down...";
    
    return 0;
}