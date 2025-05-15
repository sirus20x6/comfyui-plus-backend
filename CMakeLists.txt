cmake_minimum_required(VERSION 3.5)
project(ComfyUIPlusBackendSuperBuild LANGUAGES CXX)

# 1 Find system SQLite3 so we can hand its include/libs into the extern super‑project:
find_package(SQLite3 REQUIRED)
message(STATUS "System SQLite3 include dirs: ${SQLite3_INCLUDE_DIRS}")
message(STATUS "System SQLite3 libraries:    ${SQLite3_LIBRARIES}")

set(EP_PREFIX ${CMAKE_BINARY_DIR}/_deps)
include(ExternalProject)

# 2 Build all dependencies (hinnant_date, jwt-cpp, sqlpp11, connector) under extern/
ExternalProject_Add(dependencies_superbuild
  PREFIX            ${EP_PREFIX}/dependencies_superbuild
  SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/extern
  BINARY_DIR        ${EP_PREFIX}/dependencies_superbuild/build
  INSTALL_COMMAND   ""  # we just want to build+install into install/
  CMAKE_ARGS
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
    -DSQLITE3_INCLUDE_DIRS=${SQLite3_INCLUDE_DIRS}
    -DSQLITE3_LIBRARIES=${SQLite3_LIBRARIES}
)

# 3 Then build your app, pointing it at the install/ tree for all cmake‐config packages
ExternalProject_Add(ComfyUIPlusBackend
  DEPENDS         dependencies_superbuild
  PREFIX          ${EP_PREFIX}/main_app
  SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/app
  BINARY_DIR      ${EP_PREFIX}/main_app/build
  INSTALL_COMMAND ""
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/install
)
