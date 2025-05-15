cmake_minimum_required(VERSION 3.16) # Recommended minimum for modern ExternalProject features
project(ComfyUIPlusBackendSuperBuild LANGUAGES CXX)

# --- Set C++ Standard for this Superbuild Project (Optional, usually not critical here) ---
# This primarily affects any code directly in this CMakeLists.txt, if any.
# The actual C++ standard for dependencies and the main app are set in their respective CMakeLists.txt
# or passed via CMAKE_ARGS.
# set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
        # Pass system SQLite3 paths, as extern/CMakeLists.txt might also find_package(SQLite3)
        # or pass these directly to sqlpp23.
        -DSQLITE3_INCLUDE_DIRS=${SQLite3_INCLUDE_DIRS}
        -DSQLITE3_LIBRARIES=${SQLite3_LIBRARIES}
    # Logging for this ExternalProject step
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1 # Even with INSTALL_COMMAND "", it's good to log if it tries anything
)

# --- Main Application Build Step (manages building your actual application) ---
# This step will run the CMakeLists.txt located in the 'app' subdirectory.
ExternalProject_Add(main_app_build_step
    DEPENDS           dependencies_superbuild_step # CRUCIAL: Ensures dependencies are built first
    PREFIX            ${EP_BASE_DIR}/app_ep_prefix     # Directory for this EP step's own files
    SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/app  # Location of your app/CMakeLists.txt
    BINARY_DIR        ${EP_BASE_DIR}/app_build         # Where app/CMakeLists.txt will be configured and built
                                                     # Your final executable will be inside this directory.
    INSTALL_COMMAND   ""                               # Your app doesn't "install" from this EP step by default
                                                     # (unless app/CMakeLists.txt has install rules and this changes)
    CMAKE_ARGS        # Arguments passed to the CMake process that runs app/CMakeLists.txt
        # Propagate build type and compiler
        -DCMAKE_VERBOSE_MAKEFILE=ON
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        # Tells app/CMakeLists.txt where to find packages (HinnantDate, jwt-cpp, sqlpp23)
        # that were installed by the dependencies_superbuild_step.
        -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/install_dependencies
    # Logging for this ExternalProject step
    LOG_CONFIGURE 0
    LOG_BUILD 0
    # LOG_INSTALL 1 # Only if INSTALL_COMMAND is not ""
)

# Optional: A top-level "install" target if you want one for the whole superbuild.
# This would typically trigger the install step of main_app_build_step, which in turn
# would trigger the install rules in app/CMakeLists.txt.
# To use this, main_app_build_step's INSTALL_COMMAND would need to be, e.g.,
# INSTALL_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target install
# And app/CMakeLists.txt would need install(TARGETS ComfyUIPlusBackend ...)
#
# add_custom_target(install ALL
#     COMMAND ${CMAKE_COMMAND} --build ${EP_BASE_DIR}/app_build --target install
#     DEPENDS main_app_build_step
#     COMMENT "Installing main application"
# )