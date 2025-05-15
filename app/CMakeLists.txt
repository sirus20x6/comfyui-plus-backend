cmake_minimum_required(VERSION 3.16)
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

set(APP_SOURCES
    ${APP_MAIN_SRC}
    ${APP_CONTROLLER_SOURCES}
    ${APP_SERVICE_SOURCES}
    ${APP_MODEL_SOURCES}
    ${APP_UTIL_SOURCES}
    ${APP_FILTER_SOURCES}
)

# --- Find System Packages ---
find_package(Drogon REQUIRED)
# find_package(SQLite3 REQUIRED) # Uncomment if SQLite3 is explicitly needed
find_package(Argon2 REQUIRED)

# --- Find Dependencies built by extern/CMakeLists.txt ---
message(STATUS "app/CMakeLists.txt: CMAKE_PREFIX_PATH is: ${CMAKE_PREFIX_PATH}")

# Find date package
set(DATE_CONFIG_DIR_CANDIDATE_1 "${CMAKE_PREFIX_PATH}/hinnant_date/lib/cmake/date")
set(DATE_CONFIG_DIR_CANDIDATE_2 "${CMAKE_PREFIX_PATH}/hinnant_date/lib64/cmake/date")
if(EXISTS "${DATE_CONFIG_DIR_CANDIDATE_1}/dateConfig.cmake")
    set(date_DIR "${DATE_CONFIG_DIR_CANDIDATE_1}" CACHE INTERNAL "Path to Hinnant Date CMake config")
elseif(EXISTS "${DATE_CONFIG_DIR_CANDIDATE_2}/dateConfig.cmake")
    set(date_DIR "${DATE_CONFIG_DIR_CANDIDATE_2}" CACHE INTERNAL "Path to Hinnant Date CMake config")
else()
    message(FATAL_ERROR "Could not find dateConfig.cmake. Checked:\n"
                        "1. ${DATE_CONFIG_DIR_CANDIDATE_1}/dateConfig.cmake\n"
                        "2. ${DATE_CONFIG_DIR_CANDIDATE_2}/dateConfig.cmake")
endif()
message(STATUS "app/CMakeLists.txt: Set date_DIR to ${date_DIR}")
find_package(date REQUIRED)

# Find jwt-cpp package
set(JWT_CPP_CONFIG_DIR "${CMAKE_PREFIX_PATH}/jwt-cpp/cmake")
if(EXISTS "${JWT_CPP_CONFIG_DIR}/jwt-cpp-config.cmake")
    set(jwt-cpp_DIR "${JWT_CPP_CONFIG_DIR}" CACHE INTERNAL "Path to jwt-cpp CMake config")
else()
    message(FATAL_ERROR "jwt-cpp-config.cmake not found under ${JWT_CPP_CONFIG_DIR}")
endif()
message(STATUS "app/CMakeLists.txt: Set jwt-cpp_DIR to ${jwt-cpp_DIR}")
find_package(jwt-cpp REQUIRED)

# Find sqlpp23 package
set(SQLPP23_CMAKE_FILES_PARENT_DIR "${CMAKE_PREFIX_PATH}/sqlpp23/lib/cmake/Sqlpp23")
set(SQLPP23_POTENTIAL_NESTED_DIR "${SQLPP23_CMAKE_FILES_PARENT_DIR}/sqlpp23")

message(STATUS "app/CMakeLists.txt: Checking for Sqlpp23Config.cmake at potential nested location: ${SQLPP23_POTENTIAL_NESTED_DIR}/Sqlpp23Config.cmake")
message(STATUS "app/CMakeLists.txt: Checking for Sqlpp23Config.cmake at parent location: ${SQLPP23_CMAKE_FILES_PARENT_DIR}/Sqlpp23Config.cmake")

if(EXISTS "${SQLPP23_POTENTIAL_NESTED_DIR}/Sqlpp23Config.cmake")
    # The file is in the nested directory (e.g., .../Sqlpp23/sqlpp23/Sqlpp23Config.cmake)
    set(Sqlpp23_DIR "${SQLPP23_POTENTIAL_NESTED_DIR}" CACHE INTERNAL "Path to Sqlpp23 CMake config")
    message(STATUS "app/CMakeLists.txt: Found Sqlpp23Config.cmake in nested directory. Set Sqlpp23_DIR to ${Sqlpp23_DIR}")
elseif(EXISTS "${SQLPP23_CMAKE_FILES_PARENT_DIR}/Sqlpp23Config.cmake")
    # The file is directly in the parent directory (e.g. .../Sqlpp23/Sqlpp23Config.cmake)
    set(Sqlpp23_DIR "${SQLPP23_CMAKE_FILES_PARENT_DIR}" CACHE INTERNAL "Path to Sqlpp23 CMake config")
    message(STATUS "app/CMakeLists.txt: Found Sqlpp23Config.cmake in parent directory. Set Sqlpp23_DIR to ${Sqlpp23_DIR}")
else()
    message(FATAL_ERROR "Could not find Sqlpp23Config.cmake. Checked:\n"
                        "1. ${SQLPP23_POTENTIAL_NESTED_DIR}/Sqlpp23Config.cmake\n"
                        "2. ${SQLPP23_CMAKE_FILES_PARENT_DIR}/Sqlpp23Config.cmake")
endif()

find_package(Sqlpp23 REQUIRED) # Changed to Sqlpp23 (capital S)

# --- Target Definition ---
add_executable(${PROJECT_NAME} ${APP_SOURCES})

target_include_directories(${PROJECT_NAME}
    SYSTEM PRIVATE "${APP_INCLUDE_DIR}"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Drogon::Drogon
        Argon2::Argon2
        date::date
        jwt-cpp::jwt-cpp
        sqlpp23::sqlpp23 # Target name is still sqlpp23::sqlpp23
)

# --- Post-Build ---
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config.json"
    COMMENT "Copying config.json to app build directory"
)