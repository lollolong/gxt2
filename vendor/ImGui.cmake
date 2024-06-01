

project ("ImGui")

file(GLOB IMGUI_SOURCE_FILES
    "imgui/*.h"
    "imgui/*.cpp"
    "imgui/misc/cpp/*.*"
    "imgui/backends/imgui_impl_glfw.h"
    "imgui/backends/imgui_impl_glfw.cpp"
    "imgui/backends/imgui_impl_vulkan.h"
    "imgui/backends/imgui_impl_vulkan.cpp"
)

add_library(${PROJECT_NAME} STATIC ${IMGUI_SOURCE_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/
    )

target_link_libraries(${PROJECT_NAME} PUBLIC glfw ${Vulkan_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})