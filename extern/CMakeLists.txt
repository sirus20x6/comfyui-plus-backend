cmake_minimum_required(VERSION 3.5...3.27)
project(ExternSuperBuild LANGUAGES CXX)

include(ExternalProject)

# Find system SQLite3 for connector
find_package(SQLite3 REQUIRED)
message(STATUS "ExternSuperBuild: SQLite3 include dirs: ${SQLite3_INCLUDE_DIRS}")
message(STATUS "ExternSuperBuild: SQLite3 libraries:    ${SQLite3_LIBRARIES}")

# HinnantDate
ExternalProject_Add(
  extern_hinnant_date
  GIT_REPOSITORY   https://github.com/HowardHinnant/date.git
  GIT_TAG          v3.0.1
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install/hinnant_date
    -DBUILD_SHARED_LIBS=OFF
    -DENABLE_DATE_TESTING=OFF
    -DUSE_SYSTEM_TZ_DB=OFF
    -DBUILD_TZ_LIB=ON
)

# jwt-cpp
ExternalProject_Add(
  extern_jwt_cpp
  GIT_REPOSITORY   https://github.com/Thalhammer/jwt-cpp.git
  GIT_TAG          v0.7.0
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install/jwt-cpp
    -DBUILD_SHARED_LIBS=OFF
    -DJWT_BUILD_EXAMPLES=OFF
    -DJWT_BUILD_TESTS=OFF
)

# sqlpp11 core (tests disabled)
ExternalProject_Add(
  extern_sqlpp11
  DEPENDS          extern_hinnant_date
  GIT_REPOSITORY   https://github.com/rbock/sqlpp11.git
  GIT_TAG          0.60
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install/sqlpp11
    -DBUILD_SHARED_LIBS=OFF
    -DENABLE_TESTS=OFF
    -DENABLE_BUILD_TESTING=OFF
    -DBUILD_TESTING=OFF
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    -DBUILD_SQLPP11_PACKAGE=ON
    -Ddate_DIR=${CMAKE_BINARY_DIR}/install/hinnant_date/lib/cmake/date
)

# sqlpp11-sqlite3 connector (tests disabled)
ExternalProject_Add(
  extern_sqlpp11_sqlite3_connector
  DEPENDS          extern_sqlpp11 extern_hinnant_date
  GIT_REPOSITORY   https://github.com/rbock/sqlpp11-connector-sqlite3.git
  GIT_TAG          0.30
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install/sqlpp11-connector-sqlite3
    -DBUILD_SHARED_LIBS=OFF
    -DENABLE_TESTS=OFF
    -DENABLE_BUILD_TESTING=OFF
    -DBUILD_TESTING=OFF
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    -DSQLITE3_INCLUDE_DIR=${SQLite3_INCLUDE_DIRS}
    -DSQLITE3_LIBRARY=${SQLite3_LIBRARIES}
    -DSQLPP11_INCLUDE_DIR=${CMAKE_BINARY_DIR}/install/sqlpp11/include
    -Dsqlpp11_DIR=${CMAKE_BINARY_DIR}/install/sqlpp11/lib/cmake/sqlpp11
    -Ddate_DIR=${CMAKE_BINARY_DIR}/install/hinnant_date/lib/cmake/date
)

add_custom_target(BuildExternals ALL
  DEPENDS
    extern_hinnant_date
    extern_jwt_cpp
    extern_sqlpp11
    extern_sqlpp11_sqlite3_connector
)
add_library(superbuild_dummy INTERFACE)
