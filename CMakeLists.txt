#
# CMakeLists.txt
#

cmake_minimum_required(VERSION 3.6)

# Global
set(PROJECT_NAME gxt2)
project(${PROJECT_NAME} VERSION 1.0)

# Configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configurations" FORCE)

# No shared libs
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Shared libs")

# Properties
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Runtime
if (MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MT>
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:Release>:/MT>
    )
endif()

# Directories
add_subdirectory(src)