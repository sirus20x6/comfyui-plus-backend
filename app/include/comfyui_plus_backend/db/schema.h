#pragma once

#include <sqlite_orm/sqlite_orm.h>
#include "comfyui_plus_backend/db/models.h"
#include <string>

namespace comfyui_plus_backend
{
namespace app
{
namespace db
{

/**
 * Creates and returns a storage object for the database.
 * 
 * @param dbPath Path to the SQLite database file
 * @return A configured sqlite_orm storage object
 */
inline auto createStorage(const std::string& dbPath) {
    using namespace sqlite_orm;
    using namespace models;
    
    return make_storage(
        dbPath,
        // Users table
        make_table("users",
            make_column("id", &User::id, primary_key(), autoincrement()),
            make_column("username", &User::username, unique()),
            make_column("email", &User::email, unique()),
            make_column("hashed_password", &User::hashedPassword),
            make_column("created_at", &User::createdAt),
            make_column("updated_at", &User::updatedAt)
        ),
        
        // Workflows table
        make_table("workflows",
            make_column("id", &Workflow::id, primary_key(), autoincrement()),
            make_column("user_id", &Workflow::userId),
            make_column("name", &Workflow::name),
            make_column("description", &Workflow::description),
            make_column("json_data", &Workflow::jsonData),
            make_column("thumbnail_path", &Workflow::thumbnailPath),
            make_column("created_at", &Workflow::createdAt),
            make_column("updated_at", &Workflow::updatedAt),
            make_column("is_public", &Workflow::isPublic),
            foreign_key(&Workflow::userId).references(&User::id).on_delete.cascade()
        ),
        
        // Tags table
        make_table("tags",
            make_column("id", &Tag::id, primary_key(), autoincrement()),
            make_column("name", &Tag::name, unique())
        ),
        
        // Junction table for workflow_tags (many-to-many relationship)
        make_table("workflow_tags",
            make_column("id", &WorkflowTag::id, primary_key(), autoincrement()),
            make_column("workflow_id", &WorkflowTag::workflowId),
            make_column("tag_id", &WorkflowTag::tagId),
            foreign_key(&WorkflowTag::workflowId).references(&Workflow::id).on_delete.cascade(),
            foreign_key(&WorkflowTag::tagId).references(&Tag::id).on_delete.cascade(),
            unique(&WorkflowTag::workflowId, &WorkflowTag::tagId) // Ensure a tag is only added once to a workflow
        )
    );
}

// Define a type alias for the Storage for use throughout the application
using Storage = decltype(createStorage(""));

} // namespace db
} // namespace app
} // namespace comfyui_plus_backend