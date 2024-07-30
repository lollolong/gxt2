#
# CMakeLists.txt
#

# CMake
cmake_minimum_required(VERSION 3.27.9)

if(CMAKE_HOST_WIN32)
	set(CMAKE_SYSTEM_VERSION 10.0)
endif(CMAKE_HOST_WIN32)

set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)
project(gxt2 VERSION 1.1)

# Options
set(BUILD_SHARED_LIBS OFF)
option(GXT2_ENABLE_UNITY_BUILD "Enable Unity-Build type for gxt2" OFF)
option(GXT2_IS_APPIMAGE_BUILD "Signals gxt2 that it is being built as an AppImage" OFF)

# Properties
if(MSVC)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT gxt2edit)
endif(MSVC)

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

# Includes
include(cmake/glfw.cmake)
include(cmake/json.cmake)
include(cmake/imgui.cmake)
include(cmake/pfd.cmake)

# Directories
add_subdirectory(${VENDOR_DIR})
add_subdirectory(src)