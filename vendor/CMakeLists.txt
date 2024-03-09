#
# vendor/CMakeLists.txt
#

# GLFW
set(GLFW_INSTALL OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
add_subdirectory(GLFW)

# ImGui
set(SOURCES
	imgui/imgui.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
	imgui/misc/cpp/imgui_stdlib.cpp
	imgui/backends/imgui_impl_glfw.cpp
	imgui/backends/imgui_impl_vulkan.cpp
	
	imgui/imgui.h
	imgui/imconfig.h
	imgui/imgui_internal.h
	imgui/misc/cpp/imgui_stdlib.h
	imgui/backends/imgui_impl_glfw.h
	imgui/backends/imgui_impl_vulkan.h
)
set(IMGUI_INCLUDE
	# vendor
	${VENDOR_DIR}/imgui
	${VENDOR_DIR}/imgui/backends
	${VENDOR_DIR}/glfw/include
	${VENDOR_DIR}/rapidjson/include
	
	# Vulkan
	${Vulkan_INCLUDE_DIRS}
)
include_directories(${IMGUI_INCLUDE})
add_library(ImGui STATIC ${SOURCES})

# move projects
set(DEPENDENCIES_FOLDER "Dependencies")

set_target_properties(glfw PROPERTIES FOLDER ${DEPENDENCIES_FOLDER})
set_target_properties(ImGui PROPERTIES FOLDER ${DEPENDENCIES_FOLDER})
set_target_properties(update_mappings PROPERTIES FOLDER ${DEPENDENCIES_FOLDER})