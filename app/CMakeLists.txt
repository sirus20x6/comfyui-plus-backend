cmake_minimum_required(VERSION 3.10)
project(ComfyUIPlusBackend LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Source Files & Include Directories ---
set(APP_MAIN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/main.cc")
set(APP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(APP_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE APP_CONTROLLER_SOURCES "${APP_SRC_DIR}/controllers/*.cc")
file(GLOB_RECURSE APP_SERVICE_SOURCES "${APP_SRC_DIR}/services/*.cc")
file(GLOB_RECURSE APP_MODEL_SOURCES "${APP_SRC_DIR}/models/*.cc")
file(GLOB_RECURSE APP_UTIL_SOURCES "${APP_SRC_DIR}/utils/*.cc")
file(GLOB_RECURSE APP_FILTER_SOURCES "${APP_SRC_DIR}/filters/*.cc")
file(GLOB_RECURSE APP_DB_SOURCES "${APP_SRC_DIR}/db/*.cc")

set(APP_SOURCES
    ${APP_MAIN_SRC}
    ${APP_CONTROLLER_SOURCES}
    ${APP_SERVICE_SOURCES}
    ${APP_MODEL_SOURCES}
    ${APP_UTIL_SOURCES}
    ${APP_FILTER_SOURCES}
    ${APP_DB_SOURCES}
)

# --- Find System Packages ---
# Find SQLite3 first to avoid conflicts with Drogon's internal finder
find_package(SQLite3 REQUIRED)

# Set a variable to prevent Drogon from finding SQLite3 again
set(DROGON_USING_EXTERNAL_SQLITE3 TRUE CACHE INTERNAL "")
find_package(Drogon REQUIRED)

find_package(Argon2 REQUIRED)

# --- Find Dependencies built by extern/CMakeLists.txt ---
message(STATUS "app/CMakeLists.txt: CMAKE_PREFIX_PATH is: ${CMAKE_PREFIX_PATH}")

# Find jwt-cpp package
set(JWT_CPP_CONFIG_DIR "${CMAKE_PREFIX_PATH}/jwt-cpp/cmake")
if(EXISTS "${JWT_CPP_CONFIG_DIR}/jwt-cpp-config.cmake")
    set(jwt-cpp_DIR "${JWT_CPP_CONFIG_DIR}" CACHE INTERNAL "Path to jwt-cpp CMake config")
else()
    message(FATAL_ERROR "jwt-cpp-config.cmake not found under ${JWT_CPP_CONFIG_DIR}")
endif()
message(STATUS "app/CMakeLists.txt: Set jwt-cpp_DIR to ${jwt-cpp_DIR}")
find_package(jwt-cpp REQUIRED)

# Find sqlite_orm
find_package(SqliteOrm QUIET)

if(NOT SQLITE_ORM_FOUND)
    message(STATUS "Using embedded sqlite_orm approach as fallback...")
    # Create a source directory for the embedded header
    set(EMBEDDED_SQLITE_ORM_DIR "${CMAKE_CURRENT_BINARY_DIR}/embedded_sqlite_orm")
    file(MAKE_DIRECTORY "${EMBEDDED_SQLITE_ORM_DIR}/include/sqlite_orm")
    
    # Download the header directly
    file(DOWNLOAD
        "https://raw.githubusercontent.com/fnc12/sqlite_orm/master/include/sqlite_orm/sqlite_orm.h"
        "${EMBEDDED_SQLITE_ORM_DIR}/include/sqlite_orm/sqlite_orm.h"
        STATUS DOWNLOAD_STATUS
        TIMEOUT 30
        TLS_VERIFY ON
    )
    
    # Check download success
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    if(STATUS_CODE EQUAL 0)
        set(SQLITE_ORM_INCLUDE_DIR "${EMBEDDED_SQLITE_ORM_DIR}/include")
        set(SQLITE_ORM_FOUND TRUE)
        
        # Create custom target
        add_library(SQLITE_ORM::SqliteOrm INTERFACE IMPORTED)
        set_target_properties(SQLITE_ORM::SqliteOrm PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SQLITE_ORM_INCLUDE_DIR}"
        )
        
        message(STATUS "Successfully downloaded sqlite_orm.h to ${SQLITE_ORM_INCLUDE_DIR}")
    else()
        list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
        message(FATAL_ERROR "Failed to download sqlite_orm: ${ERROR_MESSAGE}")
    endif()
endif()

message(STATUS "app/CMakeLists.txt: Found sqlite_orm include dir: ${SQLITE_ORM_INCLUDE_DIR}")

# --- Target Definition ---
add_executable(${PROJECT_NAME} ${APP_SOURCES})

target_include_directories(${PROJECT_NAME}
    SYSTEM PRIVATE
        "${APP_INCLUDE_DIR}"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Drogon::Drogon
        ${SQLite3_LIBRARIES}  # Use the variable from find_package
        Argon2::Argon2
        jwt-cpp::jwt-cpp
        SQLITE_ORM::SqliteOrm
)

# --- Post-Build ---
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config.json"
    COMMENT "Copying config.json to app build directory"
)