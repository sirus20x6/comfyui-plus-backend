#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <vector>
#include <deque>
#include <iostream>  // Added for std::cerr
#include <algorithm> // Added for std::find
#include <sqlite_orm/sqlite_orm.h>
#include "comfyui_plus_backend/db/simple_storage.h"

namespace comfyui_plus_backend
{
namespace app
{
namespace services
{

/**
 * @brief A simple database connection pool for SQLite
 * 
 * This class manages a pool of database connections that can be
 * reused across different requests to improve performance.
 */
class DbConnectionPool
{
public:
    // Get the singleton instance
    static DbConnectionPool& getInstance() {
        static DbConnectionPool instance;
        return instance;
    }

    // Initialize the pool with SQLite connections
    bool initSqlitePool(const std::string& dbPath, size_t poolSize, bool debug = false) {
        std::lock_guard<std::mutex> lock(poolMutex_);
        
        if (initialized_) {
            return true; // Already initialized
        }
        
        try {
            // Create the initial connections
            for (size_t i = 0; i < poolSize; ++i) {
                auto storage = std::make_shared<db::Storage>(db::createStorage(dbPath));
                
                // Synchronize schema to create tables if they don't exist
                if (i == 0) {
                    storage->sync_schema();
                }
                
                connectionPool_.push_back(storage);
            }
            
            dbPath_ = dbPath;
            debugMode_ = debug;
            initialized_ = true;
            return true;
        }
        catch (const std::exception& e) {
            // Log the error
            if (debug) {
                // Use drogon logging if available, otherwise std::cerr
                std::cerr << "Error initializing connection pool: " << e.what() << std::endl;
            }
            return false;
        }
    }

    // Get a connection from the pool
    std::shared_ptr<db::Storage> getConnection() {
        std::lock_guard<std::mutex> lock(poolMutex_);
        
        if (!initialized_ || connectionPool_.empty()) {
            if (debugMode_) {
                std::cerr << "Connection pool not initialized or empty" << std::endl;
            }
            
            // If there are no connections, try to create a new one
            if (!dbPath_.empty()) {
                try {
                    auto storage = std::make_shared<db::Storage>(db::createStorage(dbPath_));
                    return storage;
                }
                catch (const std::exception& e) {
                    if (debugMode_) {
                        std::cerr << "Error creating new connection: " << e.what() << std::endl;
                    }
                    return nullptr;
                }
            }
            
            return nullptr;
        }
        
        // Get a connection from the pool
        auto connection = connectionPool_.front();
        connectionPool_.pop_front();
        
        // Return the connection to be used
        usedConnections_.push_back(connection);
        return connection;
    }

    // Return a connection to the pool
    void returnConnection(std::shared_ptr<db::Storage> connection) {
        std::lock_guard<std::mutex> lock(poolMutex_);
        
        // Find and remove from used connections
        auto it = std::find(usedConnections_.begin(), usedConnections_.end(), connection);
        if (it != usedConnections_.end()) {
            usedConnections_.erase(it);
            connectionPool_.push_back(connection);
        }
    }

    // Check if pool is initialized
    bool isInitialized() const {
        return initialized_;
    }

    // Get the current pool size
    size_t getPoolSize() const {
        std::lock_guard<std::mutex> lock(poolMutex_);
        return connectionPool_.size();
    }

    // Get the number of used connections
    size_t getUsedConnectionCount() const {
        std::lock_guard<std::mutex> lock(poolMutex_);
        return usedConnections_.size();
    }

private:
    // Private constructor for singleton pattern
    DbConnectionPool() : initialized_(false), debugMode_(false) {}
    
    // Private destructor
    ~DbConnectionPool() {
        // Clear the pools to release connections
        connectionPool_.clear();
        usedConnections_.clear();
    }
    
    // Delete copy constructor and assignment operator
    DbConnectionPool(const DbConnectionPool&) = delete;
    DbConnectionPool& operator=(const DbConnectionPool&) = delete;

    // Connection pools
    std::deque<std::shared_ptr<db::Storage>> connectionPool_;
    std::vector<std::shared_ptr<db::Storage>> usedConnections_;
    
    // Database path for creating new connections if needed
    std::string dbPath_;
    
    // Mutex for thread safety
    mutable std::mutex poolMutex_;
    
    // Initialization flag
    bool initialized_;
    
    // Debug mode flag
    bool debugMode_;
};

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend