// app/include/comfyui_plus_backend/db_schema/Users.h
#pragma once

#include <sqlpp23/core/basic/table.h>
#include <sqlpp23/core/basic/column.h>
#include <sqlpp23/core/name/char_sequence.h> // For sqlpp::text
#include <sqlpp23/core/type_traits/data_type.h>    // For sqlpp::integer, sqlpp::time_point etc.
#include <sqlpp23/core/name/name_tag.h>      // For SQLPP_CREATE_NAME_TAG

namespace Schema // Or your chosen namespace
{
  // Forward declaration of the table for column definitions
  struct Users;

  namespace Users_ // Underscore to avoid name clashes, a common convention
  {
    // Using SQLPP_CREATE_NAME_TAG for explicit C++ name vs. SQL name mapping if needed
    // If C++ name and SQL column name are the same (after case conversion, often to snake_case),
    // you might not need SQLPP_CREATE_NAME_TAG for that column.
    // sqlpp23 is more flexible with naming.

    SQLPP_COLUMN(id, ::sqlpp::integer, Users, ::sqlpp::constraints::primary_key_t<::sqlpp::constraints::auto_increment_t>);
    SQLPP_COLUMN(username, ::sqlpp::text, Users, ::sqlpp::constraints::unique_t<::sqlpp::constraints::not_null_t>);
    SQLPP_COLUMN(email, ::sqlpp::text, Users, ::sqlpp::constraints::unique_t<::sqlpp::constraints::not_null_t>);
    SQLPP_COLUMN(hashed_password, ::sqlpp::text, Users, ::sqlpp::constraints::not_null_t); // SQL name: hashed_password
    SQLPP_COLUMN(created_at, ::sqlpp::time_point, Users, ::sqlpp::constraints::default_t<decltype(::sqlpp::current_timestamp)>); // SQL name: created_at
    SQLPP_COLUMN(updated_at, ::sqlpp::time_point, Users, ::sqlpp::constraints::default_t<decltype(::sqlpp::current_timestamp)>); // SQL name: updated_at
  }

  struct Users : sqlpp::table_t<Users,
                                Users_::id,
                                Users_::username,
                                Users_::email,
                                Users_::hashed_password,
                                Users_::created_at,
                                Users_::updated_at>
  {
    // Helper for accessing table name, often not strictly needed with sqlpp23's direct usage
    static constexpr auto _alias = sqlpp::name_tag_t<"users">{};
    using _traits = sqlpp::make_traits<::sqlpp::integer, ::sqlpp::tag::is_primary_key, ::sqlpp::tag::is_autoincrement>; // For id
    // Add more traits if needed for other columns
  };
} // namespace Schema