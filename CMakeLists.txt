#
# CMakeLists.txt
#

# CMake
cmake_minimum_required(VERSION 3.6)

# Global
set(PROJECT_NAME gxt2)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)
project(${PROJECT_NAME} VERSION 1.0)

# Configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# Options
set(BUILD_SHARED_LIBS OFF)

# Properties
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Vulkan
find_package(Vulkan)
if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
	set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
	STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()

# Runtime
if (MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MT>
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:Release>:/MT>
    )
endif()

# Directories
add_subdirectory(${VENDOR_DIR})
add_subdirectory(src)