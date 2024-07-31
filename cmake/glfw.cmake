include(FetchContent)

set(GLFW_INSTALL OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        7b6aead9fb88b3623e3b3725ebb42670cbe4c579
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(glfw)

set_target_properties(glfw PROPERTIES FOLDER "Dependencies")
set_target_properties(update_mappings PROPERTIES FOLDER "Dependencies")

if(GXT2_ENABLE_UNITY_BUILD)
	set_target_properties(glfw PROPERTIES UNITY_BUILD ON)
	set_target_properties(glfw PROPERTIES UNITY_BUILD_BATCH_SIZE 16)
endif(GXT2_ENABLE_UNITY_BUILD)