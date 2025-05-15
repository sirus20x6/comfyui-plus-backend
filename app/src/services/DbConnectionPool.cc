#include "comfyui_plus_backend/services/DbConnectionPool.h"
#include <drogon/drogon.h> // For logging
#include <expected>        // C++23 feature for error handling
#include <source_location> // C++20 feature for better error reporting
#include <print>           // C++23 feature for formatted output

namespace comfyui_plus_backend
{
namespace app
{
namespace services
{

// Initialize static members
std::mutex DbConnectionPool::initMutex_;

DbConnectionPool& DbConnectionPool::getInstance()
{
    // Still thread-safe in C++11 and later
    static DbConnectionPool instance;
    return instance;
}

bool DbConnectionPool::initSqlitePool(const std::string& dbPath, size_t initialSize, bool debugMode)
{
    // Use std::lock_guard with C++17 deduction guides
    std::lock_guard lock(initMutex_);
    
    // Check if already initialized
    if (sqlitePoolInitialized_) {
        LOG_WARN << "DbConnectionPool: SQLite pool already initialized. Ignoring reinit attempt.";
        return true;
    }
    
    try {
        // Configure the SQLite connection
        sqliteConfig_.path_to_database = dbPath;
        sqliteConfig_.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        
        // Set up debug logging if enabled - using C++23 lambda improvements
        if (debugMode) {
            sqliteConfig_.debug = sqlpp::debug_logger{
                {sqlpp::log_category::all},
                [](std::string_view msg) { 
                    // Use C++23 std::print for internal formatting if you want
                    LOG_DEBUG << "[sqlpp23] " << msg; 
                }
            };
        }
        
        // Create the connection pool
        sqlitePool_ = std::make_shared<sqlpp::sqlite3::connection_pool>(sqliteConfig_, initialSize);
        sqlitePoolInitialized_ = true;
        
        // Test a connection to verify it works
        auto testConn = sqlitePool_->get();
        if (!testConn) {
            LOG_ERROR << "DbConnectionPool: Failed to get test connection after pool initialization";
            sqlitePoolInitialized_ = false;
            return false;
        }
        
        LOG_INFO << "DbConnectionPool: SQLite connection pool initialized successfully with "
                 << initialSize << " connections";
        return true;
    }
    catch (const sqlpp::exception& e) {
        auto loc = std::source_location::current();
        LOG_ERROR << "DbConnectionPool: Failed to initialize SQLite pool (sqlpp error) at " 
                 << loc.file_name() << ":" << loc.line() << ": " << e.what();
        sqlitePoolInitialized_ = false;
        return false;
    }
    catch (const std::exception& e) {
        auto loc = std::source_location::current();
        LOG_ERROR << "DbConnectionPool: Failed to initialize SQLite pool at "
                 << loc.file_name() << ":" << loc.line() << ": " << e.what();
        sqlitePoolInitialized_ = false;
        return false;
    }
}

// Using C++23's std::expected for better error handling
std::expected<std::shared_ptr<sqlpp::sqlite3::connection>, std::string> 
DbConnectionPool::getSqliteConnectionEx()
{
    if (!sqlitePoolInitialized_) {
        return std::unexpected("SQLite pool not initialized");
    }
    
    try {
        auto conn = sqlitePool_->get();
        if (!conn) {
            return std::unexpected("Got null connection from pool");
        }
        return conn;
    }
    catch (const sqlpp::exception& e) {
        return std::unexpected(std::string("sqlpp error: ") + e.what());
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("Standard error: ") + e.what());
    }
}

std::shared_ptr<sqlpp::sqlite3::connection> DbConnectionPool::getSqliteConnection()
{
    if (!sqlitePoolInitialized_) {
        LOG_ERROR << "DbConnectionPool: Attempting to get connection from uninitialized SQLite pool";
        return nullptr;
    }
    
    try {
        auto conn = sqlitePool_->get();
        return conn;
    }
    catch (const sqlpp::exception& e) {
        auto loc = std::source_location::current();
        LOG_ERROR << "DbConnectionPool: Failed to get SQLite connection (sqlpp error) at "
                 << loc.file_name() << ":" << loc.line() << ": " << e.what();
        return nullptr;
    }
    catch (const std::exception& e) {
        auto loc = std::source_location::current();
        LOG_ERROR << "DbConnectionPool: Failed to get SQLite connection at "
                 << loc.file_name() << ":" << loc.line() << ": " << e.what();
        return nullptr;
    }
}

bool DbConnectionPool::isSqlitePoolInitialized() const
{
    return sqlitePoolInitialized_;
}

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend