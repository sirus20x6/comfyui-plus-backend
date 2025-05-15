cmake_minimum_required(VERSION 3.5...3.27)
project(ExternSuperBuild LANGUAGES CXX)

include(ExternalProject)

# System SQLite3 (for connector usage)
find_package(SQLite3 REQUIRED)
message(STATUS "ExternSuperBuild: SQLite3 include dirs: ${SQLite3_INCLUDE_DIRS}")
message(STATUS "ExternSuperBuild: SQLite3 libraries:    ${SQLite3_LIBRARIES}")

# HinnantDate (date utility library)
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

# jwt-cpp (JSON Web Tokens)
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

# sqlpp23 core (C++23 SQL EDSL with built-in connectors)
ExternalProject_Add(
  extern_sqlpp23
  GIT_REPOSITORY   https://github.com/rbock/sqlpp23.git
  GIT_TAG          main
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install/sqlpp23
    -DBUILD_SHARED_LIBS=OFF
    -DENABLE_TESTS=OFF
    -DENABLE_BUILD_TESTING=OFF
    -DBUILD_TESTING=OFF
)

# Aggregate target to build all externals
add_custom_target(BuildExternals ALL
  DEPENDS
    extern_hinnant_date
    extern_jwt_cpp
    extern_sqlpp23
)
add_library(superbuild_dummy INTERFACE)
