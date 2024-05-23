//
//	grc/glfw_vulkan.h
//

#ifndef _GLFW_VULKAN_H_
#define _GLFW_VULKAN_H_

#ifdef UI_BACKEND

// imgui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <misc/cpp/imgui_stdlib.h>

// glfw
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// vulkan
#include <vulkan/vulkan.h>

// C/C++
#include <string>
#include <vector>
#include <cassert>

// Debug and Assert
#ifdef _DEBUG
	#define VULKAN_DEBUG
	#define ASSERT_VULKAN(vkRes) assert(vkRes == VK_SUCCESS);
#else
	#define ASSERT_VULKAN(vkRes) (void)vkRes;
#endif

using namespace std;

class CGraphics
{
public:
	static bool Init(const string& windowTitle, int width, int height);
	static void Shutdown();

	static void PreRender();
	static void Render();

	static bool IsRunning();

private:

	// Init
	static void InitVulkan();
	static void InitVulkanWindow();
	static void InitPhysicalDevice();
	static void InitLogicalDevice();
	static void InitDescriptorPool();

	// Cleanup
	static void CleanupVulkan();
	static void CleanupVulkanWindow();

	// Render
	static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* drawData);
	static void FramePresent(ImGui_ImplVulkanH_Window* wd);

	// ImGui
	static void InitImGui();
	static void SetupFonts();
	static void SetupTheme();
public:
	static GLFWwindow* sm_Window;
	static VkInstance sm_VulkanInstance;
	static VkPhysicalDevice sm_PhysicalDevice;
	static VkDevice sm_Device;
	static uint32_t sm_QueueFamily;
	static VkQueue sm_Queue;
	static VkDescriptorPool sm_DescriptorPool;

	static ImGui_ImplVulkanH_Window sm_MainWindowData;
	static int sm_MinImageCount;
	static bool sm_SwapChainRebuild;
};

#endif // UI_BACKEND

#endif // _GLFW_VULKAN_H_