#pragma once

#include "comfyui_plus_backend/db/models.h"
#include "comfyui_plus_backend/db/schema.h"
#include <sqlite_orm/sqlite_orm.h>
#include <string>
#include <memory>
#include <mutex>
#include <thread>

namespace comfyui_plus_backend
{
namespace app
{
namespace db
{

/**
 * @brief A thread-safe manager for database connections
 * 
 * This class provides centralized database connection management
 * and supports thread-safe access patterns for sqlite_orm.
 */
class DatabaseManager
{
public:
    // Get the singleton instance
    static DatabaseManager& getInstance();

    // Initialize with database path - must be called before any other method
    bool initialize(const std::string& dbPath);

    // Get a thread-local storage connection
    Storage& getStorage();

    // Migration helpers - runs DDL scripts
    bool migrateDatabase();

    // Check if database is initialized
    bool isInitialized() const;

    // Delete copy constructor and assignment operator to ensure singleton pattern
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

private:
    // Private constructor for singleton pattern
    DatabaseManager();
    
    // Destructor to clean up resources
    ~DatabaseManager();

    // Create tables if they don't exist
    void syncSchema();

    // Main storage for the primary thread - for simple single-threaded access
    std::unique_ptr<Storage> mainStorage_;

    // Thread-local storage map for multithreaded access
    struct ThreadLocalData {
        std::unique_ptr<Storage> storage;
    };
    static thread_local ThreadLocalData threadLocalData_;

    // Database path
    std::string dbPath_;

    // Mutex for initialization protection
    std::mutex initMutex_;

    // Flag indicating if the database has been initialized
    bool initialized_ = false;
};

} // namespace db
} // namespace app
} // namespace comfyui_plus_backend