# comfyui-plus-backend/extern/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
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

# --- HinnantDate ---
ExternalProject_Add(
  extern_hinnant_date
  PREFIX           ${DEPS_EP_TEMP_BUILD_ROOT}/hinnant_date_prefix # Temp files for this EP
  SOURCE_DIR       ${DEPS_EP_TEMP_BUILD_ROOT}/hinnant_date_src    # Source checkout
  BINARY_DIR       ${DEPS_EP_TEMP_BUILD_ROOT}/hinnant_date_bld    # Where HinnantDate itself builds
  GIT_REPOSITORY   https://github.com/HowardHinnant/date.git
  GIT_TAG          v3.0.1
  CMAKE_ARGS       # Args for HinnantDate's own CMake process
    # HinnantDate will install its files into: <build/install_dependencies>/hinnant_date
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/hinnant_date
    -DBUILD_SHARED_LIBS=OFF -DENABLE_DATE_TESTING=OFF -DUSE_SYSTEM_TZ_DB=OFF -DBUILD_TZ_LIB=ON
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/hinnant_date # Where EP expects its installed files
  LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
)

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

# --- sqlpp23 core ---
ExternalProject_Add(
  extern_sqlpp23
  PREFIX           ${DEPS_EP_TEMP_BUILD_ROOT}/sqlpp23_prefix
  SOURCE_DIR       ${DEPS_EP_TEMP_BUILD_ROOT}/sqlpp23_src
  BINARY_DIR       ${DEPS_EP_TEMP_BUILD_ROOT}/sqlpp23_bld
  DEPENDS          extern_hinnant_date
  GIT_REPOSITORY   https://github.com/rbock/sqlpp23.git
  GIT_TAG          main # Still recommend a specific tag
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/sqlpp23
    -DBUILD_SHARED_LIBS=OFF -DENABLE_TESTS=OFF -DBUILD_SQLPP23_PACKAGE=ON
    -Ddate_DIR=${CMAKE_INSTALL_PREFIX}/hinnant_date/lib/cmake/date # Path to HinnantDate's CMake config
    -DSQLPP23_WITH_SQLITE3=ON
    -DSQLITE3_INCLUDE_DIR=${SQLite3_INCLUDE_DIRS}
    -DSQLITE3_LIBRARY=${SQLite3_LIBRARIES}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/sqlpp23
  LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
)

add_custom_target(BuildExternals ALL
  DEPENDS extern_hinnant_date extern_jwt_cpp extern_sqlpp23
)
add_library(extern_dependencies_dummy INTERFACE)