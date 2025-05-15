# sqlpp23 Public API Documentation

This document outlines the public API of the sqlpp23 library, focusing on elements intended for direct use by developers.

## Core Concepts

### Data Types
Represents SQL data types. User-defined types can also be created.
- `sqlpp::boolean`
- `sqlpp::integral`
- `sqlpp::unsigned_integral`
- `sqlpp::floating_point`
- `sqlpp::text`
- `sqlpp::blob`
- `sqlpp::day_point` (from `<sqlpp23/core/chrono.h>`)
- `sqlpp::time_of_day` (from `<sqlpp23/core/chrono.h>`)
- `sqlpp::time_point` (from `<sqlpp23/core/chrono.h>`)
- `sqlpp::numeric` (generic numeric type)
- `std::optional<DataType>` for nullable types.

**Chrono Utilities (`<sqlpp23/core/chrono.h>`)**
- `sqlpp::chrono::day_point`: Alias for `std::chrono::time_point<std::chrono::system_clock, std::chrono::days>`.
- `sqlpp::chrono::microsecond_point`: Alias for `std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>`.
- `sqlpp::chrono::time_of_day(TimePoint)`: Extracts time of day as `std::chrono::microseconds`.

### Aliases and Name Tags
Used to name tables, columns, and expressions in SQL.
- `SQLPP_CREATE_NAME_TAG(NAME)`: Macro to create a name tag `NAME_t` and an instance `NAME`.
- `SQLPP_CREATE_QUOTED_NAME_TAG(NAME)`: Macro for names that require quoting.
- Predefined alias tags in `sqlpp::alias`: `a`, `b`, ..., `z`, `left`, `right`.

### Tables and Columns
- Tables are typically defined using DDL-to-C++ generators. They provide member columns.
  - `table.as(alias)`: Creates a table alias.
- `sqlpp::column_t<Table, ColumnSpec>`: Represents a column (usually accessed via table members).
  - `column = value`: Creates an assignment expression.
- `sqlpp::all_of(table)`: Selects all columns from a table.
- `sqlpp::star`: Represents `*` in select statements.

## Connection Management

### Configuration (`<sqlpp23/sqlite3/database/connection_config.h>`)
- **`sqlpp::sqlite3::connection_config`**: Struct to configure SQLite3 connections.
  - Members: `path_to_database` (std::string), `flags` (int), `vfs` (std::string), `password` (std::string), `debug` (sqlpp::debug_logger).

### Connection (`<sqlpp23/sqlite3/database/connection.h>`)
- **`sqlpp::sqlite3::connection`**: Typedef for `sqlpp::normal_connection<sqlpp::sqlite3::connection_base>`.
  - `connection(config)`: Constructor.
  - `operator()(statement)`: Executes a complete SQL statement.
  - `operator()(std::string_view)`: Executes a raw SQL string.
  - `prepare(statement)`: Prepares a statement.
  - `start_transaction()`
  - `start_transaction(isolation_level)`
  - `commit_transaction()`
  - `rollback_transaction()`
  - `is_transaction_active()`: bool
  - `last_insert_id()`: uint64_t
  - `attach(config, name)`: Attaches another database.
  - `escape(std::string_view)`: Escapes a string for SQL.
  - `set_default_isolation_level(isolation_level)`
  - `get_default_isolation_level()`: sqlpp::isolation_level
  - `is_connected()`: bool
  - `ping_server()`: bool

- **`sqlpp::sqlite3::pooled_connection`**: Typedef for `sqlpp::pooled_connection<sqlpp::sqlite3::connection_base>`. (API similar to `connection` but obtained from a pool).

### Connection Pool (`<sqlpp23/sqlite3/database/connection_pool.h>`)
- **`sqlpp::sqlite3::connection_pool`**: Manages a pool of SQLite3 connections.
  - `connection_pool(config, capacity)`: Constructor.
  - `get(connection_check check = connection_check::passive)`: Retrieves a connection from the pool.
  - `initialize(config, capacity)`

### Transactions (`<sqlpp23/core/database/transaction.h>`)
- **`sqlpp::transaction_t<Db>`**: RAII wrapper for transactions.
  - `commit()`
  - `rollback()`
- **`sqlpp::start_transaction(db)`**: Begins a transaction.
- **`sqlpp::start_transaction(db, isolation_level)`**: Begins a transaction with a specific isolation level.
- **`sqlpp::isolation_level`**: Enum ( `undefined`, `serializable`, `repeatable_read`, `read_committed`, `read_uncommitted`).

### Exception Handling (`<sqlpp23/core/database/exception.h>`)
- **`sqlpp::exception`**: Standard exception type for library errors.

## Statement Builders & Clauses
Statements are built by chaining clause methods.

### Common Statement Operations
- Most statement builder functions return a statement object. Clauses are added by calling methods on this object.
- `dynamic(condition, expression)`: Makes an expression or clause conditional.

### SELECT Statements (`<sqlpp23/core/clause/select.h>`)
- `sqlpp::select(...)`: Starts a SELECT statement.
  - `.flags(sqlpp::distinct, ...)`: Adds flags like `DISTINCT`, `ALL`.
    - `sqlpp::distinct`, `sqlpp::all`, `sqlpp::no_flag`
  - `.columns(col1, col2.as(alias::a), ...)`: Specifies columns to select.
  - `.from(table)`
  - `.join(table).on(condition)`
  - `.inner_join(table).on(condition)`
  - `.left_outer_join(table).on(condition)`
  - `.right_outer_join(table).on(condition)` (SQLite specific: requires version >= 3.39.0)
  - `.full_outer_join(table).on(condition)` (SQLite specific: requires version >= 3.39.0)
  - `.cross_join(table)`
  - `.where(condition)`
  - `.group_by(expr1, expr2, ...)`
  - `.having(condition)`
  - `.order_by(expr.asc(), expr2.desc(), ...)`
  - `.limit(count)`
  - `.offset(count)`
  - `.union_all(select_statement)`
  - `.union_distinct(select_statement)`
  - `.for_update()` (Availability depends on SQL backend)
  - `.as(alias)`: Turns the SELECT statement into a named subquery (table source).

### INSERT Statements
- **Core (`<sqlpp23/core/clause/insert.h>`)**
  - `sqlpp::insert()`: Starts a generic INSERT statement.
  - `sqlpp::insert_into(table)`: Starts an INSERT statement targeting `table`.
    - `.columns(col1, col2, ...)`: Specifies columns for which values will be provided row-wise.
      - `.add_values(val1, val2, ...)`: Adds a row of values.
    - `.set(col1 = val1, col2 = val2, ...)`: Sets values for columns.
    - `.default_values()`: Inserts default values for all columns.
- **SQLite Specific (`<sqlpp23/sqlite3/clause/insert.h>`, `<sqlpp23/sqlite3/clause/insert_or.h>`)**
  - `sqlpp::sqlite3::insert()`: Starts an SQLite3 INSERT statement.
  - `sqlpp::sqlite3::insert_into(table)`
    - `.on_conflict(...)` (SQLite specific: requires version >= 3.35.0 for full support)
      - `.do_nothing()`
      - `.do_update(col1 = val1, ...).where(condition)`
    - `.returning(col1, col2.as(alias::a), ...)` (SQLite specific: requires version >= 3.35.0)
  - `sqlpp::sqlite3::insert_or_replace()`: Starts `INSERT OR REPLACE`.
  - `sqlpp::sqlite3::insert_or_ignore()`: Starts `INSERT OR IGNORE`.
    - (Followed by `.into(table).columns(...)` or `.into(table).set(...)`)

### UPDATE Statements
- **Core (`<sqlpp23/core/clause/update.h>`)**
  - `sqlpp::update(table)`: Starts an UPDATE statement.
    - `.set(col1 = val1, col2 = sqlpp::default_value, ...)`
    - `.where(condition)`
- **SQLite Specific (`<sqlpp23/sqlite3/clause/update.h>`)**
  - `sqlpp::sqlite3::update(table)`
    - `.returning(col1, col2.as(alias::a), ...)` (SQLite specific: requires version >= 3.35.0)

### DELETE Statements
- **Core (`<sqlpp23/core/clause/delete_from.h>`)**
  - `sqlpp::delete_from()`: Starts a DELETE statement.
  - `sqlpp::delete_from(table)`: Starts a DELETE statement targeting `table`.
    - `.where(condition)`
- **SQLite Specific (`<sqlpp23/sqlite3/clause/delete_from.h>`)**
  - `sqlpp::sqlite3::delete_from()`
  - `sqlpp::sqlite3::delete_from(table)`
    - `.using_(other_table)` (Note: SQLite3 generally does not support `USING` in `DELETE` directly in the same way as PostgreSQL. This might map to a subquery or have specific SQLite syntax implications.)
    - `.returning(col1, col2.as(alias::a), ...)` (SQLite specific: requires version >= 3.35.0)

### TRUNCATE Statements (`<sqlpp23/core/clause/truncate.h>`)
- `sqlpp::truncate(table)`: Starts a TRUNCATE statement. (SQLite3 maps this to `DELETE FROM table`).

### WITH Clause (Common Table Expressions) (`<sqlpp23/core/clause/with.h>`)
- `sqlpp::with(cte1, cte2, ...)`: Starts a statement with CTEs.
  - `cte(name_tag).as(select_statement)`: Defines a CTE.
    - `cte.as(alias)`: Aliases a CTE for use in joins.
    - `cte.union_all(select_statement)` / `cte.union_distinct(select_statement)`: For recursive CTEs.

### Expression Grouping and Chaining
- CRTP Bases for enabling fluent APIs:
  - `sqlpp::enable_as`: For expressions that can be aliased (`.as(alias)`).
  - `sqlpp::enable_comparison`: For expressions that can be used in comparisons and sorting.
  - `sqlpp::enable_join`: For table-like expressions that can be joined.
  - `sqlpp::enable_over`: For aggregate functions that can use an OVER clause.

## Expressions

### Literals and Parameters
- `sqlpp::value(literal)`: Wraps a C++ literal (e.g., int, string) as an SQL value.
- `sqlpp::parameter(column_type)` or `sqlpp::parameter(data_type, name_tag_provider)`: Represents a runtime parameter.
- `sqlpp::default_value`: Represents the SQL `DEFAULT` keyword.
- `std::nullopt`: Represents SQL `NULL`.

### Column Access
- Typically `table_alias.column_name`.

### Operators (`<sqlpp23/core/operator.h>`)
- **Arithmetic**: `+`, `-` (binary and unary), `*`, `/`, `%`
  - Applied to numeric expressions. `+` is also overloaded for text concatenation.
- **Comparison**: `==`, `!=`, `<`, `<=`, `>`, `>=`
  - `expr.is_null()`, `expr.is_not_null()`
  - `expr.is_distinct_from(other_expr)`, `expr.is_not_distinct_from(other_expr)`
  - `expr.like(pattern_expr)`
  - `expr.between(lower_bound, upper_bound)`
  - `expr.in(value1, value2, ...)` or `expr.in(std::vector<value_type>)` or `expr.in(select_statement)`
  - `expr.not_in(...)`
- **Logical**: `and`, `or`, `!` (for `NOT`)
- **Bitwise**: `&`, `|`, `^` (XOR), `~` (NOT), `<<`, `>>`
- **Assignment**: `column = value` (used in `SET` clauses).

### Functions (`<sqlpp23/core/function.h>`)

#### Aggregate Functions
- `sqlpp::avg(expression)`
- `sqlpp::avg(sqlpp::distinct, expression)`
- `sqlpp::count(expression)` or `sqlpp::count(sqlpp::star)`
- `sqlpp::count(sqlpp::distinct, expression)`
- `sqlpp::max(expression)`
- `sqlpp::max(sqlpp::distinct, expression)`
- `sqlpp::min(expression)`
- `sqlpp::min(sqlpp::distinct, expression)`
- `sqlpp::sum(expression)`
- `sqlpp::sum(sqlpp::distinct, expression)`
- `aggregate_function.over()`: For window functions.

#### Scalar Functions
- `sqlpp::concat(expr1, expr2, ...)`: Concatenates text expressions.
- `sqlpp::current_date`
- `sqlpp::current_time`
- `sqlpp::current_timestamp`
- `sqlpp::lower(text_expression)`
- `sqlpp::upper(text_expression)`
- `sqlpp::trim(text_expression)`
- `sqlpp::exists(select_statement)`
- `sqlpp::any(select_statement)`: Used with comparison operators (e.g., `column == sqlpp::any(...)`). (SQLite3 does not support `ANY()`).
- `sqlpp::case_when(condition).then(result_expr).else_(else_expr)`
- `sqlpp::flatten(context, expression)`: Converts an expression to a verbatim string (parameters not allowed).
- `sqlpp::verbatim<DataType>(sql_string)`: Inserts a raw SQL string, typed as `DataType`.
- `sqlpp::parameterized_verbatim<DataType>(sql_prefix, expression, sql_suffix)`

## Result Handling

### Result Object (`<sqlpp23/core/result.h>`)
- **`sqlpp::result_t<DbResult, ResultRow>`**: Container for query results, iterable.
  - `begin()`, `end()`: Iterators.
  - `front()`: Access the current row (if not empty).
  - `empty()`: Checks if there are no (more) rows.
  - `pop_front()`: Advances to the next row.
  - `size()`: (If supported by connector) Number of rows.

### Result Row (`<sqlpp23/core/query/result_row.h>`)
- **`sqlpp::result_row_t<FieldSpecs...>`**: Represents a single row from a query result.
  - Columns are accessed by their (aliased) name: `row.column_alias`.
  - `operator bool()`: True if the row is valid.

### Prepared Statements
- Returned by `db.prepare(statement)`.
- **`sqlpp::prepared_select_t<Db, Statement>`**
- **`sqlpp::prepared_insert_t<Db, Statement>`**
- **`sqlpp::prepared_update_t<Db, Statement>`**
- **`sqlpp::prepared_delete_t<Db, Statement>`**
- **`sqlpp::prepared_execute_t<Db, Statement>`**
  - Public member: `params` (of `_parameter_list_t` type).
    - Set parameter values: `prep.params.parameter_name = value;`
  - Executed via `db(prepared_statement)`.

**SQLite3 Prepared Statement (`<sqlpp23/sqlite3/prepared_statement.h>`)**
- `sqlpp::sqlite3::prepared_statement_t`: SQLite3 specific prepared statement handle.

**SQLite3 Result Binding (`<sqlpp23/sqlite3/bind_result.h>`)**
- `sqlpp::sqlite3::bind_result_t`: Handles result set iteration and data extraction for SQLite3.

## Utilities

### Dynamic Queries (`<sqlpp23/core/query/dynamic.h>`)
- `sqlpp::dynamic(condition, expression)`: Conditionally includes an expression or clause.
- `sqlpp::dynamic(std::optional<expression>)`: Includes the expression if the optional has a value.
  If an expression is dynamically excluded, its SQL rendering often defaults to a sensible value (e.g., `NULL AS alias` for a select column, or simply omitted for a `WHERE` clause component).

### Debugging (`<sqlpp23/core/debug_logger.h>`)
- `sqlpp::debug_enabled`: `constexpr bool` indicating if debugging is compiled in.
- `sqlpp::log_category`: Enum for categorizing log messages.
- `sqlpp::debug_logger`: Class to configure and use logging.
  - `debug_logger(categories, log_function)`: Constructor.
  - `log(category, format_string, args...)`: Logs a message if the category is enabled.

### Schema Support
- `sqlpp::schema(name_string)`: Creates a schema object.
- `sqlpp::schema_qualified_table(schema_object, table_object)`: Qualifies a table with a schema.

### SQLite3 Version Constraints (`<sqlpp23/sqlite3/constraints.h>`)
- `ANY()`: Not supported by SQLite3.
- `USING` in `DELETE` or `UPDATE`: Not directly supported in the same way as some other DBs.
- `FULL OUTER JOIN`: Requires SQLite >= 3.39.0.
- `RIGHT OUTER JOIN`: Requires SQLite >= 3.39.0.
- `RETURNING` clause: Requires SQLite >= 3.35.0.
- `ON CONFLICT` (full support): Requires SQLite >= 3.35.0.
- `WITH` clause (CTE): Requires SQLite >= 3.8.3.

This API documentation provides a starting point. For detailed usage, specific connector features, and advanced patterns, refer to the library's examples and source code.