sqlpp23 Documentation (Condensed)
1. Setup
1.1. Backends
Supports MySQL, MariaDB, SQLite3, SQLCipher, PostgreSQL.
1.2. CMake Integration
FetchContent (Recommended): No installation needed.
FindPackage: Requires prior installation.
Provides targets: sqlpp23::sqlpp23 (core), sqlpp23::mysql, sqlpp23::mariadb, sqlpp23::sqlite3, sqlpp23::sqlcipher, sqlpp23::postgresql.
1.3. Manual Build & Install (if not using FetchContent)
cmake -B build <options>
cmake --build build --target install
Options:
BUILD_MYSQL_CONNECTOR=ON, etc.
DEPENDENCY_CHECK=OFF (to install connectors even if dependencies are missing system-wide).
MSVC users: Define NOMINMAX preprocessor macro when using MySQL backend.
1.4. Code Generation (ddl2cpp)
Generates C++ table representations from DDL files.
Obtain DDL (e.g., mysqldump --no-data MyDatabase > MyDatabase.sql).
Run script: %sqlpp23_dir%/scripts/ddl2cpp <input_ddl_file> <output_cpp_header_stem> <namespace_for_tables>
2. Core Concepts
2.1. Data Types
Nullable: std::optional<T>. SQL NULL is std::nullopt.
Bool: SQL BOOL -> sqlpp::boolean (column spec). C++: bool (params/results).
Integer: SQL INT/BIGINT -> sqlpp::integral. C++: int64_t. (int8_t through int64_t are sqlpp::integral).
Unsigned Integer: SQL UNSIGNED INT -> sqlpp::unsigned_integral. C++: uint64_t. (uint8_t through uint64_t are sqlpp::unsigned_integral).
Floating Point: SQL FLOAT/DOUBLE -> sqlpp::floating_point. C++: double. (float, double, long double are sqlpp::floating_point).
Text: SQL CHAR/VARCHAR/TEXT -> sqlpp::text. Params: std::string. Results: std::string_view. (char, const char*, std::string, std::string_view are sqlpp::text).
Blob: SQL BLOB -> sqlpp::blob. Params: std::vector<uint8_t>. Results: std::span<uint8_t>. (std::array<uint8_t>, std::span<uint8_t>, std::vector<uint8_t> are sqlpp::blob).
Date: SQL DATE -> sqlpp::day_point. C++: std::chrono::time_point<std::chrono::system_clock, std::chrono::days>.
Datetime: SQL DATETIME -> sqlpp::time_point. C++: std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>.
Time of day: SQL TIME -> sqlpp::time_of_day. C++: std::chrono::microseconds. (std::chrono::duration<Rep, Period> is sqlpp::time_of_day).
Timezones: Ignored. Recommended: Use UTC in DB and C++.
2.2. NULL Handling
SQL NULL comparison: expr = NULL results in NULL. sqlpp23 preserves this.
Operators:
expr.is_null()
expr.is_not_null()
expr.is_distinct_from(value_or_nullopt) (like std::optional::operator!=)
expr.is_not_distinct_from(value_or_nullopt) (like std::optional::operator==)
These support parameters and std::nullopt.
2.3. Dynamic Statements with dynamic()
Modify statement structure at runtime using dynamic(condition, expression).
If condition is false:
Selected column dynamic(cond, col): Serializes to NULL AS name; result row member is std::nullopt.
Joined table dynamic(cond, table): Table not joined.
Condition fragment dynamic(cond, where_expr): Fragment omitted.
set assignment dynamic(cond, assignment): Assignment omitted.
order_by expression dynamic(cond, order_expr): Expression omitted.
group_by expression dynamic(cond, group_expr): Expression omitted.
limit/offset value dynamic(cond, value): Clause omitted.
on_conflict target dynamic(cond, target): Target omitted.
Validation: Reduced for dynamic parts. Static components cannot depend on dynamic components unless also dynamic.
2.4. Debug Logging
Configuration: connection_config->debug of type sqlpp::debug_logger.
Default: No logging.
sqlpp::debug_logger(categories, log_function):
categories: std::vector<sqlpp::log_category>. Enum sqlpp::log_category: statement, parameter, result, connection, all.
log_function: std::function<void(const std::string&)>.
// Log all to stderr
config->debug = sqlpp::debug_logger{{sqlpp::log_category::all}, [](const auto& msg){ std::cerr << msg << '\n'; }};
// Turn off
config->debug = {};
Use code with caution.
C++
Compile-time disable: #define SQLPP23_DISABLE_DEBUG before including sqlpp23/core/debug_logger.h.
2.5. Names and Aliases
Create name tags for aliases: SQLPP_CREATE_NAME_TAG(my_alias_name); (at namespace scope).
Create quoted name tags: SQLPP_CREATE_QUOTED_NAME_TAG(my_quoted_alias);.
Use in expressions: expr.as(my_alias_name).
Sub-selects as tables require aliasing: select(...).as(my_alias_name).
2.6. Exceptions
sqlpp::exception primarily thrown by connectors.
Expect when: Connecting, preparing statements, executing statements, retrieving/iterating results.
3. Statement Execution
3.1. Connection
Create config: auto config = std::make_shared<sqlpp::connector::connection_config>(); ...
Connect: sqlpp::connector::connection db; db.connect_using(config); (can throw).
MySQL specific: Call sqlpp::mysql::global_library_init() once.
3.2. Executing Statements
Use operator() on connection object: db(statement_or_string).
String execution: db("CREATE TABLE ..."); returns affected rows/status.
sqlpp23 statement execution:
// Returns affected rows for DML
size_t affected_rows = db(delete_from(tab).where(tab.id == 17));
// Returns last insert id for some INSERTs (connector dependent)
int64_t last_insert_id = db(insert_into(tab).default_values());
// Returns a result object for SELECT
for (const auto& row : db(select(tab.id).from(tab))) { /* ... */ }
Use code with caution.
C++
Compile-time checks: Invalid statements cause static_assert.
3.3. Prepared Statements
Prepare:
SQLPP_CREATE_NAME_TAG(param_name); // At namespace scope
auto prepared_stmt = db.prepare(
    insert_into(tab).set(
        tab.col1 = parameter(tab.col1), // Use column as template for type and name
        tab.col2 = parameter(sqlpp::text(), param_name) // Explicit type and name tag
));
Use code with caution.
C++
Set parameters: prepared_stmt.params.col1 = value; prepared_stmt.params.param_name = "text_value";
Execute: db(prepared_stmt);
3.4. Result Rows (from SELECT)
Iterate with range-based for loop: for (const auto& row : db(select_stmt)).
Access fields: row.column_alias. Typed, std::optional for nullable.
TEXT fields: std::string_view. BLOB fields: std::span<uint8_t>.
Programmatic access: row.as_tuple().
std::ranges::views can process results.
Alternative iteration: while(!result.empty()) { const auto& row = result.front(); ... result.pop_front(); }.
4. Tables and Joins
4.1. Table Representation
Generated by ddl2cpp from DDL.
constexpr auto my_table = schema::TableName{};
Columns are members: my_table.column_name.
4.2. Joins
table1.join_type(table2).on(condition)
Join types:
join (alias inner_join)
left_outer_join
right_outer_join
full_outer_join (MySQL: not supported, compile error. SQLite3: not supported before 3.39.0)
cross_join (no on condition)
Chainable: table1.join(table2).on(...) .left_join(table3).on(...)
Unconditional cross join: foo.cross_join(bar) (replaces sqlpp11 foo.join(bar).unconditionally()).
4.3. Aliased Tables
table_object.as(alias_name_tag) or cte_object.as(alias_name_tag).
SQLPP_CREATE_NAME_TAG(left_alias);
auto l = foo.as(left_alias);
4.4. Common Table Expressions (CTEs)
Non-recursive: const auto my_cte = cte(sqlpp::alias::my_cte_name).as(select_statement);
Recursive:
const auto x_base = cte(sqlpp::alias::x).as(select(sqlpp::value(0).as(sqlpp::alias::a)));
const auto x_cte = x_base.union_all(select((x_base.a + 1).as(sqlpp::alias::a)).from(x_base).where(x_base.a < 10));
Use code with caution.
C++
Use with with clause: with(my_cte) << select(...)
SQLite3: with not supported before 3.8.0.
5. SQL Statements
5.1. SELECT
for (const auto& row :
     db(select(foo.id, (foo.value + 10).as(aliased_value))
            .flags(sqlpp::distinct) // Optional: sqlpp::all (default), sqlpp::distinct
            .from(foo.join(bar).on(foo.bar_id == bar.id))
            .where(foo.name.like("%suffix") and dynamic(maybe, bar.type == "active"))
            .group_by(foo.category)
            .having(count(foo.id) > 1)
            .order_by(foo.id.desc())
            .limit(10)
            .offset(20)
            .for_update() // Optional: Adds FOR UPDATE
        )) {
    // use row.id, row.aliased_value
}
Use code with caution.
C++
select(...): Takes named expressions. Use .as(alias) for expressions or duplicate names.
all_of(table): Selects all columns from a table.
Mixing aggregates (e.g. max()) and non-aggregates: Non-aggregates must be in group_by. If group_by is used, all selected columns must be aggregates or group_by columns.
flags(): sqlpp::all (default), sqlpp::distinct. Can be dynamic.
from(): Table, aliased table, sub-select with alias, join, or CTE. Can be dynamic.
where(): Boolean condition. Can be dynamic. If dynamic condition false, no WHERE clause.
group_by(): Expressions. Arguments can be dynamic.
having(): Boolean condition (usually with aggregates). Can be dynamic.
order_by(): expr.asc(), expr.desc(). Arguments can be dynamic.
limit(), offset(): Integer arguments. Can be dynamic.
for_update(): Adds simplified "FOR UPDATE".
5.2. INSERT
Single row:
db(insert_into(tab).set(tab.col1 = "value", tab.col2 = std::nullopt, tab.col3 = sqlpp::default_value));
Use code with caution.
C++
Must assign to all non-nullable columns without defaults.
set arguments can be dynamic. If all dynamic args false, statement invalid.
Default values for all columns: db(insert_into(tab).default_values());
Multi-row insert:
auto multi_insert = insert_into(tab).columns(tab.col1, tab.col2);
multi_insert.values.add(tab.col1 = "valA1", tab.col2 = 10);
multi_insert.values.add(tab.col1 = "valA2", tab.col2 = 20);
db(multi_insert);
Use code with caution.
C++
add() requires precise value types (cast if needed, e.g., for std::chrono).
returning clause: See Section 5.7.
on_conflict: See Section 5.8.
SQLite3: insert_or_replace(), insert_or_ignore().
db(sqlpp::sqlite3::insert_or_replace().into(tab).set(tab.col = "val"));
Use code with caution.
C++
5.3. UPDATE
db(update(tab)
    .set(tab.col1 = "new_value", tab.col2 = tab.col2 + 1, dynamic(maybe_update, tab.col3 = "dyn_val"))
    .where(tab.id == 1));
Use code with caution.
C++
update(table_object).
set(): Assignments. LHS must be columns of the updated table. Args can be dynamic. If all dynamic args false, statement invalid.
where(): Optional. Specifies affected rows. Can be dynamic.
returning clause: See Section 5.7.
MySQL: Supports order_by() and limit() in update.
sqlpp::mysql::update(tab).set(tab.boolN = true).order_by(tab.intN.desc()).limit(1);
Use code with caution.
C++
5.4. DELETE
db(delete_from(tab).where(tab.category == "old"));
Use code with caution.
C++
where(): Optional. Specifies affected rows. Can be dynamic.
returning clause: See Section 5.7.
PostgreSQL & MySQL: Support using clause.
// PostgreSQL example
sqlpp::postgresql::delete_from(tab)
    .using_(other_tab)
    .where(tab.other_id == other_tab.id and other_tab.criteria == val);
Use code with caution.
C++
MySQL: Supports order_by() and limit() in delete_from.
SQLite3: delete_from ... using not supported before 3.39.0.
5.5. TRUNCATE
Deletes all rows from a table.
db(truncate(tab));
SQLite3: Serializes to conditionless DELETE FROM tab;.
5.6. WITH (Common Table Expressions)
See Section 4.4.
Used to provide CTEs to a main statement (usually SELECT, but also INSERT, UPDATE, DELETE in some DBs).
with(cte1, cte2) << select(...)
(sqlpp11: with(cte1)(select(...)))
5.7. RETURNING Clause
Supported by insert_into, update, delete_from to return columns from affected rows.
Available for PostgreSQL, SQLite3.
// PostgreSQL example
for (const auto& row :
     db(sqlpp::postgresql::update(foo)
            .set(foo.intN = 0)
            .returning(foo.id, dynamic(maybe, foo.textN)))) {
  // use row.id, row.textN (std::optional if dynamic)
}
Use code with caution.
C++
5.8. ON CONFLICT (Upsert)
Primarily for PostgreSQL and SQLite3.
5.8.1. ON CONFLICT ... DO NOTHING
// No conflict targets (PostgreSQL, SQLite3)
db(insert_into(foo).default_values().on_conflict().do_nothing());

// With conflict targets (PostgreSQL, SQLite3)
// For SQLite3, typically one target like a primary/unique key.
// For PostgreSQL, can be multiple columns or constraint name.
db(insert_into(foo)
       .set(foo.col = val)
       .on_conflict(foo.id, dynamic(maybe, foo.unique_col)) // foo.unique_col dynamic for PG
       .do_nothing());
Use code with caution.
C++
sqlpp23 does not validate conflict targets against DB constraints.
5.8.2. ON CONFLICT ... DO UPDATE
// PostgreSQL, SQLite3
for (const auto& row :
     db(insert_into(foo)
        .set(foo.col1 = initial_val1, foo.col2 = initial_val2)
        .on_conflict(foo.id) // Conflict target(s)
        .do_update( // Assignments for the UPDATE path
            foo.col1 = excluded.col1, // 'excluded' refers to values proposed for insertion
            dynamic(maybe, foo.col2 = "updated_value")
        )
        .where(foo.some_existing_col == condition) // Optional: condition on existing row
        .returning(foo.id) // Optional: if supported
    )) {
    // use row.id
}
Use code with caution.
C++
The exact syntax for referring to excluded values (excluded.col or database-specific variant) is handled by the connector.
5.9. Custom Query Parts
Concatenate clauses using operator<< for statements not directly supported.
Example for SELECT ... INTO (syntax may vary by DB):
// sqlpp11: custom_query(select(...), into(...))
// sqlpp23:
select(all_of(t)).from(t)
    << into(f) // 'into' would be a custom serializable part
    << with_result_type_of(insert_into(f)); // If it affects rows like an insert
Use code with caution.
C++
6. Expressions and Operators
6.1. Aggregate Functions
Perform calculations across rows, often with group_by. Require .as(alias) in select.
avg(expr) / avg(sqlpp::distinct, expr): Numeric arg, result nullable float.
count(expr_or_star) / count(sqlpp::distinct, expr): sqlpp::star for COUNT(*). Result integer.
max(expr) / max(sqlpp::distinct, expr): Comparable arg. Result nullable equivalent of arg.
min(expr) / min(sqlpp::distinct, expr): Comparable arg. Result nullable equivalent of arg.
sum(expr) / sum(sqlpp::distinct, expr): Numeric arg. Result nullable equivalent of arg.
aggregate_func(expr).over(): Window function. Returns aggregate for each row. over() currently takes no arguments.
max(foo.id).over().as(max_id_per_row)
6.2. IN / NOT IN
expr.in(value1, value2, ...)
expr.in(std::vector<type> values) (sqlpp11 required sqlpp::value_list(vector))
expr.in(select_statement_returning_one_column)
expr.not_in(...) (same argument types)
Empty vector behavior: x.in({}) is FALSE, x.not_in({}) is TRUE. (sqlpp11 had different "magic" behavior).
6.3. NULL-safe comparisons
See Section 2.2. is_distinct_from, is_not_distinct_from.
6.4. Other Operators
Standard arithmetic (+, -, *, /), comparison (==, !=, <, >, <=, >=), logical (and, or, not) operators are available.
text_col.like("pattern%").
7. Sub-Selects
7.1. As a Value
Single column, single row sub-select can be used where a value is expected.
...where(foo.text == select(bar.text).from(bar).where(bar.id == foo.id))
7.2. As a Selected Value
Wrap in sqlpp::value() and alias with .as(alias).
select(all_of(foo), value(select(bar.text)...).as(bar_text_value)).from(foo)
7.3. As a Value Set (with IN, ANY, EXISTS)
Single column sub-select.
...where(foo.id.in(select(bar.foo_id).from(bar)))
...where(exists(select(sqlpp::star).from(bar).where(bar.foo_id == foo.id)))
7.4. As a Table
Alias the select statement using .as(alias_name_tag).
SQLPP_CREATE_NAME_TAG(sub_q_alias);
auto sub_query_table = select(foo.id, foo.name).from(foo).where(...).as(sub_q_alias);
// Use in another query:
db(select(sub_query_table.id).from(sub_query_table));
Use code with caution.
C++
8. Transactions
auto tx = start_transaction(db); // Default isolation level
// auto tx = start_transaction(db, sqlpp::isolation_level::serializable);
// auto tx = start_transaction(db, sqlpp::quiet_auto_rollback); // Suppress report on auto-rollback
try {
    db(insert_into(tab)...);
    tx.commit();
} catch (const sqlpp::exception& e) {
    tx.rollback(); // Or let destructor rollback automatically
}
// If tx goes out of scope without commit/rollback, destructor calls rollback().
// This is reported by connection unless sqlpp::quiet_auto_rollback was used.
Use code with caution.
C++
9. Connectors
9.1. General
Connectors adapt statement serialization to specific SQL dialects.
Differences for is_distinct_from serialization:
MySQL: NOT (col <=> NULL)
PostgreSQL: col IS DISTINCT FROM NULL
SQLite3: col IS NOT NULL (if comparing to NULL)
9.2. PostgreSQL (sqlpp::postgresql)
Connection: config->user, config->database.
delete_from: Supports using and returning.
returning: Supported for insert_into, update, delete_from.
on_conflict().do_nothing(): 0+ conflict targets (column names or constraint name).
on_conflict().do_update(): Uses excluded table to refer to proposed insertion values.
9.3. SQLite3 and SQLCipher (sqlpp::sqlite3)
Connection: config->path_to_database, config->flags.
insert_or_replace(), insert_or_ignore().
returning: Supported for insert_into, update, delete_from.
on_conflict().do_nothing(): Typically one conflict target (e.g., PK).
on_conflict().do_update(): Uses excluded table.
Unsupported/Limitations:
any: Fails to compile.
delete_from ... using: Fails compile (before SQLite 3.39.0).
full_outer_join, right_outer_join: Fails compile (before SQLite 3.39.0).
with (CTEs): Fails compile (before SQLite 3.8.0).
truncate(): Serialized as conditionless DELETE FROM.
9.4. MySQL / MariaDB (sqlpp::mysql)
Connection: Call sqlpp::mysql::global_library_init() once. config->user, config->database.
update: Supports order_by() and limit().
delete_from: Supports using(), order_by(), and limit().
Unsupported/Limitations:
full_outer_join: Compile-time static_assert.
returning clause: Not generally supported by MySQL for DML.
on_conflict: MySQL uses ON DUPLICATE KEY UPDATE. sqlpp23 mapping might require custom syntax or specific connector functions not detailed in provided docs for this feature.
10. Advanced Topics
10.1. Thread Safety
sqlpp23 itself offers no specific thread-safety guarantees beyond the underlying client library.
Do not share connection objects between threads.
Using the same connection simultaneously in multiple threads typically leads to issues.
Use Connection Pools for concurrent database access.
10.2. Connection Pools
Cache of database connections.
Classes: sqlpp::mysql::connection_pool, sqlpp::postgresql::connection_pool, sqlpp::sqlite3::connection_pool.
Constructor: PoolType(config_ptr, initial_cache_size). Or pool.initialize(config_ptr, size).
Get connection: auto db = pool.get();
Return connection: Automatic on db destruction (RAII).
Connection check on get: pool.get(sqlpp::connection_check::type)
none: No check.
passive: (PostgreSQL only) Checks if server closed its side.
ping: Sends dummy request (mysql_ping for MySQL, SELECT 1 for others).
Usage for thread safety:
Get new connection per request: pool.get()(select(...)). Not suitable for transactions spanning multiple queries.
One connection per thread: thread_local wrapper that lazily fetches a connection from the pool.
11. Key Differences from sqlpp11 (Summary)
Simpler Syntax: std::optional for nulls, direct std::vector for IN, no .unconditionally().
Improved Type Safety: Stricter checks, standard types for result fields (std::string_view, std::span<uint8_t>).
Dynamic Queries: dynamic() embedded in statements, supporting AND/OR and nesting.
Result Access: Typed members on row objects. row.as_tuple() for programmatic access.
Logging: More flexible sqlpp::debug_logger, compile-time disable.
Clauses: delete_from (was remove_from), truncate added. WITH syntax with(c) << select(...).
Operators: is_distinct_from replaces is_equal_to_or_null.
Dropped: eval(), value_list, ppgen, sqlite2cpp.py, dynamic connector loading, read-only columns, required where constraints.
Behavior changes: Empty list in IN/NOT IN acts as SQL standard. No x += y to x = x + y translation. Aggregates require explicit naming with .as().
12. TODOs / Known Limitations (Brief)
Full C++ Module support.
Further documentation: select_as/select_ref_t nuances, cte_ref with aliases, sub-select value static/dynamic table detection with outer query context.
Missing SQL Functions: coalesce, cast, EXTRACT (date/time).
CASE statement enhancements (multiple WHEN).
PRAGMA table support (SQLite).
Asynchronous operation support.