#include <sqlpp23/core/basic/table.h>
#include <sqlpp23/core/basic/column.h>

namespace comfyui_plus_backend::app::schema {

struct Users {
  struct Id {
    static constexpr auto name  = "id";
    using value_type            = sqlpp::bigint;                // 64‑bit
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

  using _column_list = sqlpp::type_vector<
      Id, Username, Email, HashedPassword, CreatedAt, UpdatedAt>;
  static constexpr auto name = "users";
};

} // namespace comfyui_plus_backend::app::schema
