cmake_minimum_required(VERSION 3.10)
project(ExternDependencies LANGUAGES CXX) # This project name is for this sub-CMake process

include(ExternalProject)
set(CMAKE_CXX_STANDARD 23) # Matches your app's CXX_STANDARD

# This CMAKE_INSTALL_PREFIX is the one passed by the top-level superbuild:
# e.g., comfyui-plus-backend/build/install_dependencies
message(STATUS "ExternDependencies (extern/CMakeLists.txt): Effective CMAKE_INSTALL_PREFIX for deps: ${CMAKE_INSTALL_PREFIX}")

# This CMAKE_CURRENT_BINARY_DIR is the one for *this* CMake process:
# e.g., comfyui-plus-backend/build/_external_projects/deps_build/
set(DEPS_EP_TEMP_BUILD_ROOT ${CMAKE_CURRENT_BINARY_DIR}/_ep_builds_temp) # Temporary build artifacts for deps
message(STATUS "ExternDependencies (extern/CMakeLists.txt): Temp build root for individual EPs: ${DEPS_EP_TEMP_BUILD_ROOT}")

find_package(SQLite3 REQUIRED) # Should find it based on system or variables passed to this CMake process
message(STATUS "ExternDependencies (extern/CMakeLists.txt): System SQLite3 include dirs: ${SQLite3_INCLUDE_DIRS}")
message(STATUS "ExternDependencies (extern/CMakeLists.txt): System SQLite3 libraries:    ${SQLite3_LIBRARIES}")

# --- jwt-cpp ---
ExternalProject_Add(
  extern_jwt_cpp
  PREFIX           ${DEPS_EP_TEMP_BUILD_ROOT}/jwt_cpp_prefix
  SOURCE_DIR       ${DEPS_EP_TEMP_BUILD_ROOT}/jwt_cpp_src
  BINARY_DIR       ${DEPS_EP_TEMP_BUILD_ROOT}/jwt_cpp_bld
  GIT_REPOSITORY   https://github.com/Thalhammer/jwt-cpp.git
  GIT_TAG          v0.7.1 # must not change
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/jwt-cpp
    -DBUILD_SHARED_LIBS=OFF -DJWT_BUILD_EXAMPLES=OFF -DJWT_BUILD_TESTS=OFF
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/jwt-cpp
  LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
)

# --- libsql (replacing sqlpp23) ---
# Instead of building from source, let's use a simpler C++ wrapper for SQLite
ExternalProject_Add(
  extern_sqlite_orm
  PREFIX           ${DEPS_EP_TEMP_BUILD_ROOT}/sqlite_orm_prefix
  SOURCE_DIR       ${DEPS_EP_TEMP_BUILD_ROOT}/sqlite_orm_src
  BINARY_DIR       ${DEPS_EP_TEMP_BUILD_ROOT}/sqlite_orm_bld
  GIT_REPOSITORY   https://github.com/fnc12/sqlite_orm.git
  GIT_TAG          v1.8.2  # Use an appropriate tag/release
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/sqlite_orm
    -DSQLITE_ORM_ENABLE_CXX_17=OFF
    -DSQLITE_ORM_ENABLE_CXX_20=ON
    -DSQLITE_ORM_OMITS_CODECVT=ON
    -DSQLITE_ORM_ENABLE_JSON=OFF
    -DBUILD_TESTING=OFF
    -DBUILD_EXAMPLES=OFF
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/sqlite_orm
  LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
)

add_custom_target(BuildExternals ALL
  DEPENDS extern_jwt_cpp extern_sqlite_orm
)
add_library(extern_dependencies_dummy INTERFACE)