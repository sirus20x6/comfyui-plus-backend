// app/src/models/User.cc
#include "comfyui_plus_backend/models/User.h"
#include "comfyui_plus_backend/utils/DateTimeUtils.h"
#include <string>

namespace comfyui_plus_backend {
namespace app {
namespace models {

// Static method implementation to create a User from a Row
User User::fromRow(const ::drogon::orm::Row &row) {
    User user;
    if (!row["id"].isNull())
        user.id_ = row["id"].as<int64_t>();
    user.username_ = row["username"].as<std::string>();
    user.email_ = row["email"].as<std::string>();
    user.hashedPassword_ = row["hashed_password"].as<std::string>();
    
    // Handle date conversion using the utility class
    if (!row["created_at"].isNull()) {
        std::string dateStr = row["created_at"].as<std::string>();
        user.createdAt_ = utils::DateTimeUtils::dbStringToDate(dateStr);
    } else {
        user.createdAt_ = trantor::Date::now();
    }
    
    if (!row["updated_at"].isNull()) {
        std::string dateStr = row["updated_at"].as<std::string>();
        user.updatedAt_ = utils::DateTimeUtils::dbStringToDate(dateStr);
    } else {
        user.updatedAt_ = trantor::Date::now();
    }
    
    return user;
}

} // namespace models
} // namespace app
} // namespace comfyui_plus_backend