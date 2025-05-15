# comfyui-plus-backend/app/cmake/FindArgon2.cmake
find_path(Argon2_INCLUDE_DIR NAMES argon2.h
          HINTS /usr/include /usr/local/include /opt/local/include)

find_library(Argon2_LIBRARY NAMES argon2 libargon2
             HINTS /usr/lib /usr/local/lib /opt/local/lib /usr/lib/x86_64-linux-gnu) # Add common lib paths

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Argon2
                                  REQUIRED_VARS Argon2_LIBRARY Argon2_INCLUDE_DIR
                                  VERSION_VAR Argon2_VERSION_STRING) # Optional version

if(Argon2_FOUND)
  set(Argon2_LIBRARIES ${Argon2_LIBRARY})
  set(Argon2_INCLUDE_DIRS ${Argon2_INCLUDE_DIR})
  if(NOT TARGET Argon2::Argon2) # Create an imported target
    add_library(Argon2::Argon2 UNKNOWN IMPORTED)
    set_target_properties(Argon2::Argon2 PROPERTIES
                          IMPORTED_LOCATION "${Argon2_LIBRARY}"
                          INTERFACE_INCLUDE_DIRECTORIES "${Argon2_INCLUDE_DIR}")
  endif()
  message(STATUS "Found Argon2: ${Argon2_LIBRARY} (include: ${Argon2_INCLUDE_DIR})")
else()
    message(STATUS "Argon2 Not Found")
endif()

mark_as_advanced(Argon2_INCLUDE_DIR Argon2_LIBRARY)