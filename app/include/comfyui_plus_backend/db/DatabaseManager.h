#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <thread>
#include <iostream>
#include "comfyui_plus_backend/db/simple_storage.h"  // Must come BEFORE this declaration

namespace comfyui_plus_backend
{
namespace app
{
namespace db
{

/**
 * @brief Manager for database connections
 * 
 * This class manages database access, providing thread-local SQLite 
 * connections to ensure thread safety and performance.
 */
class DatabaseManager
{
public:
    // Get the singleton instance
    static DatabaseManager& getInstance();
    
    // Initialize the database
    bool initialize(const std::string& dbPath);
    
    // Get the database storage for the current thread
    // Make sure Storage is fully qualified here
    comfyui_plus_backend::app::db::Storage& getStorage();
    
    // Migrate the database schema
    bool migrateDatabase();
    
    // Check if the database is initialized
    bool isInitialized() const;

private:
    // Private constructor for singleton pattern
    DatabaseManager();
    
    // Private destructor
    ~DatabaseManager();
    
    // Delete copy constructor and assignment operator
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
    // Synchronize database schema
    void syncSchema();
    
    // Thread-local storage for per-thread database access
    struct ThreadLocalData {
        std::unique_ptr<comfyui_plus_backend::app::db::Storage> storage;
    };
    
    static thread_local ThreadLocalData threadLocalData_;
    
    // Path to the database file
    std::string dbPath_;
    
    // Main database storage
    std::unique_ptr<comfyui_plus_backend::app::db::Storage> mainStorage_;
    
    // Mutex for thread safety during initialization
    std::mutex initMutex_;
    
    // Initialization flag
    bool initialized_ = false;
};

} // namespace db
} // namespace app
} // namespace comfyui_plus_backend