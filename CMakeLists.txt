cmake_minimum_required(VERSION 3.10) # Updated minimum required version
project(ComfyUIPlusBackendSuperBuild LANGUAGES CXX)

# --- Set default CMAKE_BUILD_TYPE to Debug if not specified ---
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No CMAKE_BUILD_TYPE specified, defaulting to Debug.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()
message(STATUS "Top-Level: CMAKE_BUILD_TYPE is: ${CMAKE_BUILD_TYPE}")
# --- End default CMAKE_BUILD_TYPE ---

# --- Find System SQLite3 ---
# This is found early so its paths can be passed to the dependencies_superbuild_step
find_package(SQLite3 REQUIRED)
message(STATUS "Top-Level: System SQLite3 include dirs: ${SQLite3_INCLUDE_DIRS}")
message(STATUS "Top-Level: System SQLite3 libraries:    ${SQLite3_LIBRARIES}")

# --- Base directory for all ExternalProject working directories ---
set(EP_BASE_DIR ${CMAKE_BINARY_DIR}/_external_projects)
include(ExternalProject) # Include the module

# --- Dependencies Superbuild Step (manages building all external libraries) ---
# This step will run the CMakeLists.txt located in the 'extern' subdirectory.
ExternalProject_Add(dependencies_superbuild_step
    PREFIX            ${EP_BASE_DIR}/deps_ep_prefix    # Directory for this EP step's own files (stamp, tmp)
    SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/extern # Location of the extern/CMakeLists.txt
    BINARY_DIR        ${EP_BASE_DIR}/deps_build        # Where extern/CMakeLists.txt will be configured and built
    INSTALL_COMMAND   ""                               # Dependencies install themselves via their CMAKE_INSTALL_PREFIX
    CMAKE_ARGS        # Arguments passed to the CMake process that runs extern/CMakeLists.txt
        # Propagate build type and compiler
        -DCMAKE_VERBOSE_MAKEFILE=ON
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        # This is the crucial CMAKE_INSTALL_PREFIX for the extern/CMakeLists.txt process.
        # All dependencies built by extern/CMakeLists.txt will install their public files
        # into subdirectories of this path.
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install_dependencies
        # Pass system SQLite3 paths
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5 # removing this causes the build to fail
        -DSQLITE3_INCLUDE_DIRS=${SQLite3_INCLUDE_DIRS}
        -DSQLITE3_LIBRARIES=${SQLite3_LIBRARIES}
    # Logging for this ExternalProject step
    LOG_CONFIGURE 0
    LOG_BUILD 0
    LOG_INSTALL 0 # Even with INSTALL_COMMAND "", it's good to log if it tries anything
)

# --- Main Application Build Step (manages building your actual application) ---
# This step will run the CMakeLists.txt located in the 'app' subdirectory.
ExternalProject_Add(main_app_build_step
    DEPENDS           dependencies_superbuild_step
    PREFIX            ${EP_BASE_DIR}/app_ep_prefix
    SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/app
    BINARY_DIR        ${EP_BASE_DIR}/app_build
    INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install
    CMAKE_ARGS
        -DCMAKE_VERBOSE_MAKEFILE=ON
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5 # removing this causes the build to fail
        -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/install_dependencies
        -DCMAKE_INSTALL_PREFIX=${EP_BASE_DIR}/install
    LOG_CONFIGURE 0
    LOG_BUILD 0
    LOG_INSTALL 0
)