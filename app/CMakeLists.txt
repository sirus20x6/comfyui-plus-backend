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
# Look for libSQL first
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSQL IMPORTED_TARGET libsql)

if(LIBSQL_FOUND)
    message(STATUS "Found libSQL: ${LIBSQL_LIBRARIES}")
    set(SQL_LIBRARIES ${LIBSQL_LIBRARIES})
    set(SQL_INCLUDE_DIRS ${LIBSQL_INCLUDE_DIRS})
    add_definitions(-DUSE_LIBSQL)
else()
    message(STATUS "libSQL not found, falling back to SQLite3")
    # Find SQLite3 as a fallback
    find_package(SQLite3 REQUIRED)
    set(SQL_LIBRARIES ${SQLite3_LIBRARIES})
    set(SQL_INCLUDE_DIRS ${SQLite3_INCLUDE_DIRS})
endif()

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

# --- Handle sqlite_orm ---
# Check if include directory exists
set(SQLITE_ORM_INCLUDE_DIR "${CMAKE_PREFIX_PATH}/sqlite_orm/include")
if(EXISTS "${SQLITE_ORM_INCLUDE_DIR}/sqlite_orm/sqlite_orm.h")
    message(STATUS "Found sqlite_orm in expected install location: ${SQLITE_ORM_INCLUDE_DIR}")
else()
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
        message(STATUS "Successfully downloaded sqlite_orm.h to ${SQLITE_ORM_INCLUDE_DIR}")
    else()
        list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
        message(FATAL_ERROR "Failed to download sqlite_orm: ${ERROR_MESSAGE}")
    endif()
endif()

message(STATUS "app/CMakeLists.txt: Will use sqlite_orm include dir: ${SQLITE_ORM_INCLUDE_DIR}")

# --- Target Definition ---
add_executable(${PROJECT_NAME} ${APP_SOURCES})

target_include_directories(${PROJECT_NAME}
    SYSTEM PRIVATE
        "${APP_INCLUDE_DIR}"
        ${SQL_INCLUDE_DIRS}
        "${SQLITE_ORM_INCLUDE_DIR}"  # Add direct reference to include dir
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Drogon::Drogon
        ${SQL_LIBRARIES}
        Argon2::Argon2
        jwt-cpp::jwt-cpp
)

# --- Install Targets ---
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
    DESTINATION bin
)

install(DIRECTORY 
    "${CMAKE_CURRENT_SOURCE_DIR}/database/migrations"
    DESTINATION bin/database
    FILES_MATCHING PATTERN "*.sql"
)

# --- Post-Build ---
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config.json"
    COMMENT "Copying config.json to app build directory"
)