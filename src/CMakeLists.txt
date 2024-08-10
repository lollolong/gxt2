#
# src/CMakeLists.txt
#

#------------------ gxt2conv ------------------

project("gxt2conv")

set(SOURCES
	main/gxt2conv.cpp
	main/gxt2conv.h
	
	gxt/gxt2.cpp
	gxt/gxt2.h
	
	data/stringhash.cpp
	data/stringhash.h
	
	gxt/convert.cpp
	gxt/convert.h
	
	resources/gxt2conv.rc
	resources/resource.h
	
	system/app.cpp
	system/app.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
	# project
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(${PROJECT_NAME} PRIVATE 
	cxx_std_20
)

target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

if(GXT2_ENABLE_UNITY_BUILD)
	set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
endif(GXT2_ENABLE_UNITY_BUILD)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

#------------------ gxt2merge ------------------

project("gxt2merge")

set(SOURCES
	main/gxt2merge.cpp
	main/gxt2merge.h
	
	gxt/gxt2.cpp
	gxt/gxt2.h
	
	data/stringhash.cpp
	data/stringhash.h
	
	gxt/merge.cpp
	gxt/merge.h
	
	resources/gxt2merge.rc
	resources/resource.h
	
	system/app.cpp
	system/app.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
	# project
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(${PROJECT_NAME} PRIVATE 
	cxx_std_20
)

target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

if(GXT2_ENABLE_UNITY_BUILD)
	set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
endif(GXT2_ENABLE_UNITY_BUILD)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

#------------------ gxt2edit ------------------

project("gxt2edit")

set(SOURCES
	main/gxt2edit.cpp
	main/gxt2edit.h
	main/main.h
	
	gxt/gxt2.cpp
	gxt/gxt2.h
	
	data/stringhash.cpp
	data/stringhash.h
	
	data/util.cpp
	data/util.h
	
	grc/graphics.cpp
	grc/graphics.h
	
	grc/image.cpp
	grc/image.h
	
	resources/gxt2edit.rc
	resources/resource.h
	
	system/app.cpp
	system/app.h
)

# Determine if we are on macOS
if(APPLE)
	set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
	set(CMAKE_MACOSX_RPATH ON)

    set(MACOSX_BUNDLE_BUNDLE_NAME "GXTEdit")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.lollolong.gxt2edit")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.1")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
    set(MACOSX_BUNDLE_ICON_FILE "Resources/editor.icns")

    # Get fonts
    set(FONTS_DIR ${CMAKE_SOURCE_DIR}/fonts)
    file(GLOB FONT_FILES "${FONTS_DIR}/*.ttf")

    # Define icon and labels.txt paths
    set(ICON_PATH ${CMAKE_SOURCE_DIR}/src/resources/icons/editor.icns)
    set(LABELS_PATH ${CMAKE_SOURCE_DIR}/labels.txt)

    # Add the icon file to the target
    set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    # Add the labels file to the target
    set_source_files_properties(${LABELS_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    # Add the font files to the target
    set_source_files_properties(${FONT_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/fonts")

    # Include icon and labels.txt in the target sources
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${FONT_FILES} ${ICON_PATH} ${LABELS_PATH})

    # Set macOS-specific bundle properties
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/src/resources/Info.plist
    )
else()
    # Regular executable for non-macOS platforms
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
	# project
	${CMAKE_CURRENT_SOURCE_DIR}

	# Icon Font
	${VENDOR_DIR}/IconFontCppHeaders

	# stb
	${VENDOR_DIR}/stb
	
	# Vulkan
	${Vulkan_INCLUDE_DIRS}
)

target_compile_features(${PROJECT_NAME} PRIVATE 
	cxx_std_20
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	glfw
	imgui
	portable_file_dialogs
	nlohmann_json::nlohmann_json
	${Vulkan_LIBRARIES}
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
		UxTheme 
		Dwmapi
	)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	GLFW_INCLUDE_NONE 
	GLFW_INCLUDE_VULKAN
	UI_BACKEND
	STB_IMAGE_IMPLEMENTATION
	$<$<CONFIG:Debug>:USE_VULKAN_DEBUG_REPORT>
)

if(CMAKE_HOST_WIN32)
	target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_EXPOSE_NATIVE_WIN32)
endif(CMAKE_HOST_WIN32)

if(GXT2_ENABLE_UNITY_BUILD)
	set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
endif(GXT2_ENABLE_UNITY_BUILD)

if(GXT2_IS_APPIMAGE_BUILD)
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		IS_APPIMAGE_BUILD=1
	)
endif(GXT2_IS_APPIMAGE_BUILD)
