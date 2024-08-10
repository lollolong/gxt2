include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        8199457a7d9e453f8d3d9cadc14683fb54a858b5
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(imgui)

file(GLOB IMGUI_SOURCE_FILES
    ${imgui_SOURCE_DIR}/*.h
    ${imgui_SOURCE_DIR}/*.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/*.*
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCE_FILES})

target_include_directories(imgui PUBLIC 
	${imgui_SOURCE_DIR}
	${imgui_SOURCE_DIR}/backends/
)

target_link_libraries(imgui PUBLIC glfw ${Vulkan_LIBRARIES})
target_include_directories(imgui PUBLIC ${Vulkan_INCLUDE_DIRS})

set_target_properties(imgui PROPERTIES FOLDER "Dependencies")
target_compile_features(imgui PRIVATE cxx_std_20)
