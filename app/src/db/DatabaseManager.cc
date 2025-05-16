// app/src/db/DatabaseManager.cc
#include "comfyui_plus_backend/db/DatabaseManager.h"
#include <drogon/drogon.h>
#include <filesystem>
#include <sstream> // Added for std::stringstream

namespace comfyui_plus_backend
{
namespace app
{
namespace db
{

// Initialize the thread_local storage
thread_local DatabaseManager::ThreadLocalData DatabaseManager::threadLocalData_;

// Singleton instance getter
DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

// Constructor
DatabaseManager::DatabaseManager() : initialized_(false) {
    LOG_DEBUG << "DatabaseManager constructed";
}

// Destructor
DatabaseManager::~DatabaseManager() {
    LOG_DEBUG << "DatabaseManager destroyed";
}

bool DatabaseManager::initialize(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(initMutex_);
    
    if (initialized_) {
        LOG_INFO << "DatabaseManager already initialized";
        return true;
    }
    
    try {
        // Ensure directory exists
        auto dbDir = std::filesystem::path(dbPath).parent_path();
        if (!dbDir.empty() && !std::filesystem::exists(dbDir)) {
            std::filesystem::create_directories(dbDir);
            LOG_INFO << "Created database directory: " << dbDir.string();
        }
        
        // Initialize the main storage
        dbPath_ = dbPath;
        mainStorage_ = std::make_unique<comfyui_plus_backend::app::db::Storage>(createStorage(dbPath_));
        
        // Create tables if they don't exist
        syncSchema();
        
        initialized_ = true;
        LOG_INFO << "DatabaseManager initialized with database: " << dbPath_;
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR << "Error initializing DatabaseManager: " << e.what();
        return false;
    }
}

comfyui_plus_backend::app::db::Storage& DatabaseManager::getStorage() {
    if (!initialized_) {
        throw std::runtime_error("DatabaseManager not initialized");
    }
    
    // For multithreaded access, use thread-local storage
    if (std::this_thread::get_id() != std::thread::id()) {
        // Initialize thread-local storage if not already done
        if (!threadLocalData_.storage) {
            // Convert the thread ID to a string representation
            std::stringstream ss;
            ss << std::this_thread::get_id();
            LOG_DEBUG << "Creating thread-local database connection for thread ID: " << ss.str();
            
            threadLocalData_.storage = std::make_unique<comfyui_plus_backend::app::db::Storage>(createStorage(dbPath_));
        }
        return *threadLocalData_.storage;
    }
    
    // For the main thread, use the main storage
    return *mainStorage_;
}

bool DatabaseManager::migrateDatabase() {
    if (!initialized_) {
        throw std::runtime_error("DatabaseManager not initialized");
    }
    
    try {
        // Run migrations or database setup logic here
        // In sqlite_orm we can just call sync_schema() which we already do in initialize()
        LOG_INFO << "Database migration complete";
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR << "Error during database migration: " << e.what();
        return false;
    }
}

bool DatabaseManager::isInitialized() const {
    return initialized_;
}

void DatabaseManager::syncSchema() {
    if (!mainStorage_) {
        throw std::runtime_error("Storage not initialized");
    }
    
    // This will create tables if they don't exist, or validate them if they do
    mainStorage_->sync_schema();
    
    LOG_INFO << "Database schema synchronized";
}

} // namespace db
} // namespace app
} // namespace comfyui_plus_backend