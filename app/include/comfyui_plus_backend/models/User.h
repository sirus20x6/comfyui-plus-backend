// app/include/comfyui_plus_backend/models/User.h
#pragma once

#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <drogon/orm/Field.h>
#include <drogon/orm/Mapper.h>
#include <trantor/utils/Date.h>
#include <optional>
#include <string>

namespace comfyui_plus_backend
{
namespace app
{
namespace models
{

/**
 * @brief User model class for Drogon ORM
 */
class User
{
  public:
    User() = default;
    
    // Static method to create a User from a Row
    static User fromRow(const ::drogon::orm::Row &row);
    
    // Member variables
    std::optional<int64_t> id_;
    std::string username_;
    std::string email_;
    std::string hashedPassword_; // Renamed for clarity
    trantor::Date createdAt_;
    trantor::Date updatedAt_;
    
    // Getters and Setters
    std::optional<std::int64_t> getId() const { return id_; }
    void setId(const std::int64_t &id) { id_ = id; }

    std::string getUsername() const { return username_; }
    void setUsername(const std::string &username) { username_ = username; }

    std::string getEmail() const { return email_; }
    void setEmail(const std::string &email) { email_ = email; }

    std::string getHashedPassword() const { return hashedPassword_; }
    void setHashedPassword(const std::string &hashedPassword) { hashedPassword_ = hashedPassword; }

    trantor::Date getCreatedAt() const { return createdAt_; }
    void setCreatedAt(const trantor::Date &createdAt) { createdAt_ = createdAt; }

    trantor::Date getUpdatedAt() const { return updatedAt_; }
    void setUpdatedAt(const trantor::Date &updatedAt) { updatedAt_ = updatedAt; }
};

} // namespace models
} // namespace app
} // namespace comfyui_plus_backend