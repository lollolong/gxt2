#
# vendor/CMakeLists.txt
#

# GLFW
set(GLFW_INSTALL OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)

add_subdirectory(glfw)

include("ImGui.cmake")

add_subdirectory("json")
add_subdirectory("portable-file-dialogs")

# move projects
set(DEPENDENCIES_FOLDER "Dependencies")

set_target_properties(glfw PROPERTIES FOLDER ${DEPENDENCIES_FOLDER})
set_target_properties(ImGui PROPERTIES FOLDER ${DEPENDENCIES_FOLDER})
set_target_properties(update_mappings PROPERTIES FOLDER ${DEPENDENCIES_FOLDER})
