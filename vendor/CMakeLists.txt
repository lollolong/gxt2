#
# vendor/CMakeLists.txt
#


add_subdirectory("portable-file-dialogs")

# move projects
set(DEPENDENCIES_FOLDER "Dependencies")

set_target_properties(glfw PROPERTIES FOLDER ${DEPENDENCIES_FOLDER})
#set_target_properties(ImGui PROPERTIES FOLDER ${DEPENDENCIES_FOLDER})
set_target_properties(update_mappings PROPERTIES FOLDER ${DEPENDENCIES_FOLDER})
