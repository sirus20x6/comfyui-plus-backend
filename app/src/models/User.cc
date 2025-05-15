// app/src/models/User.cc
#include "comfyui_plus_backend/models/User.h"
#include <drogon/orm/DbClient.h> // For client operations, not strictly for model def

// If using ANET_MODEL_BEGIN/END, anet might generate initializers or you might add a constructor
namespace comfyui_plus_backend
{
namespace app
{
namespace models
{
    // Example constructor if you define one (not always needed with ORM mappers)
    User::User(const drogon::orm::Row &row) {
        if (!row["id"].isNull())
            id_ = row["id"].as<int64_t>();
        username_ = row["username"].as<std::string>();
        email_ = row["email"].as<std::string>();
        hashedPassword_ = row["hashed_password"].as<std::string>(); // Ensure column name matches
        if (!row["created_at"].isNull())
            createdAt_ = row["created_at"].as<trantor::Date>();
        if (!row["updated_at"].isNull())
            updatedAt_ = row["updated_at"].as<trantor::Date>();
    }
}
}
}