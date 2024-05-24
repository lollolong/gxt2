//
//	grc/glfw_vulkan.h
//

#ifndef _GLFW_VULKAN_H_
#define _GLFW_VULKAN_H_

#ifdef UI_BACKEND

// Windows
#include <Windows.h>

// imgui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <misc/cpp/imgui_stdlib.h>

// glfw
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

class CGraphics
{
private:
	CGraphics();
public:
	bool Init(const std::string& windowTitle, int width, int height);
	void Shutdown();

	void PreRender();
	void Render();
	bool IsRunning();

	HWND GetWin32Window() const;
	static CGraphics& GetInstance() { return sm_Instance; }
private:

	// Init
	void InitVulkan();
	void InitVulkanWindow();
	void InitPhysicalDevice();
	void InitLogicalDevice();
	void InitDescriptorPool();

	// Cleanup
	void CleanupVulkan();
	void CleanupVulkanWindow();

	// Render
	void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* drawData);
	void FramePresent(ImGui_ImplVulkanH_Window* wd);

	// ImGui
	void InitImGui();
	void SetupFonts();
	void SetupTheme();
public:
	GLFWwindow* m_Window;
	VkInstance m_VulkanInstance;
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;
	uint32_t m_QueueFamily;
	VkQueue m_Queue;
	VkDescriptorPool m_DescriptorPool;
	ImGui_ImplVulkanH_Window m_MainWindowData;
	int m_MinImageCount;
	bool m_SwapChainRebuild;

	static CGraphics sm_Instance;
};

#endif // UI_BACKEND

#endif // _GLFW_VULKAN_H_