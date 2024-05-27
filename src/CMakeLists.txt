#
# src/CMakeLists.txt
#

#------------------ gxt2conv ------------------

set(gxt2conv "gxt2conv")

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

add_executable(${gxt2conv} ${SOURCES})

target_include_directories(${gxt2conv} PRIVATE
	# project
	${CMAKE_CURRENT_SOURCE_DIR}
	
	# rapidjson
	${VENDOR_DIR}/rapidjson/include
)
target_compile_features(${gxt2conv} PRIVATE 
	cxx_std_20
)
target_compile_options(${gxt2conv} PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
set_target_properties(${gxt2conv} PROPERTIES UNITY_BUILD ON)


#------------------ gxt2edit ------------------

set(gxt2edit "gxt2edit")

set(SOURCES
	main/gxt2edit.cpp
	main/gxt2edit.h
	
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

add_executable(${gxt2edit} ${SOURCES})

target_include_directories(${gxt2edit} PRIVATE
	# project
	${CMAKE_CURRENT_SOURCE_DIR}
	
	# imgui
	${VENDOR_DIR}/imgui
	${VENDOR_DIR}/imgui/backends
	
	# glfw
	${VENDOR_DIR}/glfw/include
	
	# rapidjson
	${VENDOR_DIR}/rapidjson/include
	
	# Icon Font
	${VENDOR_DIR}/IconFontCppHeaders

	# stb
	${VENDOR_DIR}/stb
	
	# Vulkan
	${Vulkan_INCLUDE_DIRS}
)
target_compile_features(${gxt2edit} PRIVATE 
	cxx_std_20
)
target_link_libraries(${gxt2edit} PRIVATE
	glfw
	ImGui
	${Vulkan_LIBRARIES}
)
target_compile_options(${gxt2edit} PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
target_compile_definitions(${gxt2edit} PRIVATE
	GLFW_INCLUDE_NONE 
	GLFW_INCLUDE_VULKAN
	GLFW_EXPOSE_NATIVE_WIN32
	UI_BACKEND
	STB_IMAGE_IMPLEMENTATION
	$<$<CONFIG:Debug>:USE_VULKAN_DEBUG_REPORT>
)
set_target_properties(${gxt2edit} PROPERTIES UNITY_BUILD ON)