#pragma once

#include <sqlpp23/sqlite3/sqlite3.h>
#include <sqlpp23/core/basic/column.h>
#include <sqlpp23/core/type_traits.h>

namespace comfyui_plus_backend::app::schema {

struct Users {
  struct Id {
    static constexpr auto name  = "id";
    using value_type            = sqlpp::integral;  // Use integral instead of bigint
    static constexpr auto flags = sqlpp::column_flags::primary_key
                                | sqlpp::column_flags::auto_increment;
  };

  struct Username {
    static constexpr auto name  = "username";
    using value_type            = sqlpp::text;
    static constexpr auto flags = sqlpp::column_flags::not_null
                                | sqlpp::column_flags::unique;
  };

  struct Email {
    static constexpr auto name  = "email";
    using value_type            = sqlpp::text;
    static constexpr auto flags = sqlpp::column_flags::not_null
                                | sqlpp::column_flags::unique;
  };

  struct HashedPassword {
    static constexpr auto name  = "hashed_password";
    using value_type            = sqlpp::text;
    static constexpr auto flags = sqlpp::column_flags::not_null;
  };

  struct CreatedAt {
    static constexpr auto name  = "created_at";
    using value_type            = sqlpp::time_point;
    static constexpr auto flags = sqlpp::column_flags::default_;
  };

  struct UpdatedAt {
    static constexpr auto name  = "updated_at";
    using value_type            = sqlpp::time_point;
    static constexpr auto flags = sqlpp::column_flags::default_;
  };

  // Using sqlpp::meta::type_vector instead of sqlpp::type_vector
  using _column_list = sqlpp::meta::type_vector
      Id, Username, Email, HashedPassword, CreatedAt, UpdatedAt>;
  static constexpr auto name = "users";
};

} // namespace comfyui_plus_backend::app::schema