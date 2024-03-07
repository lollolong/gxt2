#
# src/CMakeLists.txt
#

# Source files
set(SOURCES
    main/main.cpp

    gxt/gxt2.cpp
    gxt/gxt2.h
	
    gxt/convert.cpp
    gxt/convert.h
	
    resources/resource.rc
    resources/resource.h
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Includes
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
	
    # vendor
    ${VENDOR_DIR}/rapidjson/include
)

# Warnings
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
