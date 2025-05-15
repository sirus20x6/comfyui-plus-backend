#pragma once

#include <string>
#include <optional>

namespace comfyui_plus_backend
{
namespace app
{
namespace db
{
namespace models
{

/**
 * @brief User model for database operations
 * 
 * This struct represents the users table in the database,
 * with fields mapping directly to table columns.
 */
struct User {
    std::optional<int64_t> id;
    std::string username;
    std::string email;
    std::string hashedPassword;
    std::string createdAt;
    std::string updatedAt;
};

/**
 * @brief Workflow model for database operations
 * 
 * This struct represents the workflows table in the database,
 * mapping ComfyUI workflow data to database columns.
 */
struct Workflow {
    std::optional<int64_t> id;
    int64_t userId;
    std::string name;
    std::string description;
    std::string jsonData;
    std::string thumbnailPath;  // Path to workflow thumbnail image
    std::string createdAt;
    std::string updatedAt;
    bool isPublic = false;
};

/**
 * @brief Tag model for database operations
 * 
 * Tags can be associated with workflows for categorization and search.
 */
struct Tag {
    std::optional<int64_t> id;
    std::string name;
};

/**
 * @brief WorkflowTag model for database operations
 * 
 * This is a junction table model to represent the many-to-many relationship
 * between workflows and tags.
 */
struct WorkflowTag {
    std::optional<int64_t> id;
    int64_t workflowId;
    int64_t tagId;
};

} // namespace models
} // namespace db
} // namespace app
} // namespace comfyui_plus_backend