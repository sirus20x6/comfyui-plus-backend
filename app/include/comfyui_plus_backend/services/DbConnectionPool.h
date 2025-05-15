#pragma once

#include <sqlpp23/sqlite3/database/connection_pool.h>
#include <sqlpp23/sqlite3/database/connection.h>
#include <memory>  // For std::shared_ptr
#include <mutex>   // For std::mutex
#include <string>  // For std::string
#include <expected> // C++23 feature for error handling

namespace comfyui_plus_backend
{
namespace app
{
namespace services
{

/**
 * @brief Singleton service for managing database connection pools.
 * 
 * This class provides centralized access to database connection pools,
 * ensuring that throughout the application only one pool is created per
 * database configuration. This improves resource usage and performance.
 */
class DbConnectionPool
{
  public:
    /**
     * @brief Get the singleton instance of the DbConnectionPool.
     * 
     * @return Reference to the singleton instance.
     */
    static DbConnectionPool& getInstance();

    /**
     * @brief Initialize the SQLite connection pool.
     * 
     * This method configures and initializes the SQLite connection pool
     * using the application configuration. It should be called once during
     * application startup.
     * 
     * @param dbPath Path to the SQLite database file.
     * @param initialSize Initial number of connections in the pool.
     * @param debugMode If true, enables debug logging for SQL operations.
     * @return true if initialization was successful, false otherwise.
     */
    bool initSqlitePool(const std::string& dbPath, size_t initialSize, bool debugMode = false);

    /**
     * @brief Get a SQLite connection from the pool.
     * 
     * @return A shared pointer to a SQLite connection, or nullptr if the pool is not initialized.
     */
    std::shared_ptr<sqlpp::sqlite3::connection> getSqliteConnection();

    /**
     * @brief Get a SQLite connection with error information using C++23's std::expected.
     * 
     * @return An expected object containing either a connection or an error message.
     */
    std::expected<std::shared_ptr<sqlpp::sqlite3::connection>, std::string> getSqliteConnectionEx();

    /**
     * @brief Check if the SQLite connection pool is initialized.
     * 
     * @return true if the pool is initialized, false otherwise.
     */
    bool isSqlitePoolInitialized() const;

    // Delete copy constructor and assignment operator to ensure singleton pattern
    DbConnectionPool(const DbConnectionPool&) = delete;
    DbConnectionPool& operator=(const DbConnectionPool&) = delete;

  private:
    // Private constructor to enforce singleton pattern
    DbConnectionPool() = default;
    
    // Destructor
    ~DbConnectionPool() = default;

    // SQLite connection pool
    std::shared_ptr<sqlpp::sqlite3::connection_pool> sqlitePool_;
    
    // SQLite connection configuration
    sqlpp::sqlite3::connection_config sqliteConfig_;
    
    // Mutex for thread-safe initialization
    static std::mutex initMutex_;
    
    // Flag to track initialization state
    bool sqlitePoolInitialized_ = false;
};

} // namespace services
} // namespace app
} // namespace comfyui_plus_backend