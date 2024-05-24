#
# CMakeLists.txt
#

# CMake
cmake_minimum_required(VERSION 3.6)
set(CMAKE_SYSTEM_VERSION 10.0)

# Project
set(PROJECT_NAME gxt2)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)
project(${PROJECT_NAME} VERSION 1.0)

# Configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# Options
set(BUILD_SHARED_LIBS OFF)

# Properties
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT gxt2edit)

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

# Unicode Charset
if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# Directories
add_subdirectory(${VENDOR_DIR})
add_subdirectory(src)