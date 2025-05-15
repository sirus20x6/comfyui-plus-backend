# FindSqliteOrm.cmake
# Find the sqlite_orm header-only library
#
# This sets the following variables:
# SQLITE_ORM_FOUND        - True if sqlite_orm is found
# SQLITE_ORM_INCLUDE_DIR  - Directory where sqlite_orm headers are located
#
# SQLITE_ORM::SqliteOrm   - Imported target for sqlite_orm (only includes)

# First, check our expected install location - this is the most likely spot
set(SQLITE_ORM_INSTALL_DIR "${CMAKE_PREFIX_PATH}/sqlite_orm")
if(EXISTS "${SQLITE_ORM_INSTALL_DIR}/include/sqlite_orm/sqlite_orm.h")
    set(SQLITE_ORM_INCLUDE_DIR "${SQLITE_ORM_INSTALL_DIR}/include")
    message(STATUS "Found sqlite_orm in expected install location: ${SQLITE_ORM_INCLUDE_DIR}")
else()
    # If not found in our install location, try system paths
    find_path(SQLITE_ORM_INCLUDE_DIR
        NAMES sqlite_orm/sqlite_orm.h
        PATHS
            ${CMAKE_PREFIX_PATH}/include
            ${CMAKE_INSTALL_PREFIX}/include
            /usr/include
            /usr/local/include
        DOC "Path to sqlite_orm include directory"
    )
endif()

# Display a custom message if not found
if(NOT SQLITE_ORM_INCLUDE_DIR)
    message(STATUS "Could not find sqlite_orm.h - check if the download succeeded")
    # Check if we can directly copy the header from a known location
    message(STATUS "Attempting to download sqlite_orm.h directly...")
    
    # Create directory if it doesn't exist
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/sqlite_orm_tmp/include/sqlite_orm")
    
    # Try to download the header file
    file(DOWNLOAD
      "https://raw.githubusercontent.com/fnc12/sqlite_orm/master/include/sqlite_orm/sqlite_orm.h"
      "${CMAKE_CURRENT_BINARY_DIR}/sqlite_orm_tmp/include/sqlite_orm/sqlite_orm.h"
      STATUS DOWNLOAD_STATUS
      TIMEOUT 30
      TLS_VERIFY ON
    )
    
    # Check if the download succeeded
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
    
    if(STATUS_CODE EQUAL 0)
        message(STATUS "Successfully downloaded sqlite_orm.h")
        set(SQLITE_ORM_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/sqlite_orm_tmp/include")
    else()
        message(STATUS "Failed to download sqlite_orm.h: ${ERROR_MESSAGE}")
    endif()
endif()

# Handle standard find_package arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SqliteOrm
    REQUIRED_VARS SQLITE_ORM_INCLUDE_DIR
)

# Create imported target if found
if(SQLITE_ORM_FOUND AND NOT TARGET SQLITE_ORM::SqliteOrm)
    add_library(SQLITE_ORM::SqliteOrm INTERFACE IMPORTED)
    set_target_properties(SQLITE_ORM::SqliteOrm PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SQLITE_ORM_INCLUDE_DIR}"
    )
endif()

# Mark as advanced
mark_as_advanced(SQLITE_ORM_INCLUDE_DIR)