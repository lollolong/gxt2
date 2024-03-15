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
#include <GLFW/glfw3.h>

// vulkan
#include <vulkan/vulkan.h>

// C/C++
#include <string>

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
	// Callbacks
	static void glfw_error_callback(int error, const char* description);
	static void check_vk_result(VkResult err);

	// Init
	static void SetupVulkan(ImVector<const char*> instance_extensions);
	static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);

	// Cleanup
	static void CleanupVulkan();
	static void CleanupVulkanWindow();

	// Render
	static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
	static void FramePresent(ImGui_ImplVulkanH_Window* wd);

	// Style
	static void SetupImGuiStyle();

private:

#ifdef USE_VULKAN_DEBUG_REPORT
	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(
		VkDebugReportFlagsEXT flags, 
		VkDebugReportObjectTypeEXT objectType, 
		uint64_t object, 
		size_t location, 
		int32_t messageCode, 
		const char* pLayerPrefix, 
		const char* pMessage, 
		void* pUserData);
#endif // USE_VULKAN_DEBUG_REPORT

	static bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension);
	static VkPhysicalDevice SetupVulkan_SelectPhysicalDevice();
public:
	static GLFWwindow* sm_Window;
	static VkAllocationCallbacks* sm_Allocator;
	static VkInstance sm_Instance;
	static VkPhysicalDevice sm_PhysicalDevice;
	static VkDevice sm_Device;
	static uint32_t sm_QueueFamily;
	static VkQueue sm_Queue;
	static VkDebugReportCallbackEXT sm_DebugReport;
	static VkPipelineCache sm_PipelineCache;
	static VkDescriptorPool sm_DescriptorPool;

	static ImGui_ImplVulkanH_Window sm_MainWindowData;
	static int sm_MinImageCount;
	static bool sm_SwapChainRebuild;
};

#endif // UI_BACKEND

#endif // _GLFW_VULKAN_H_