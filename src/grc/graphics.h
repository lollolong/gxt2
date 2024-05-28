//
//	grc/graphics.h
//

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

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
#include <stack>

// Debug and Assert
#ifdef _DEBUG
	#define VULKAN_DEBUG
	#define ASSERT_VULKAN(vkRes) CheckVulkan(vkRes);
#else
	#define ASSERT_VULKAN(vkRes) (void)vkRes;
#endif

#ifdef VULKAN_DEBUG
static void CheckVulkan(VkResult vkRes)
{
	if (vkRes != VK_SUCCESS)
	{
		printf("[vulkan] Error: VkResult = %d\n", vkRes);
	}
	assert(vkRes == VK_SUCCESS);
}

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
	bool IsRunning() const;
	bool IsMinimized() const;

	HWND GetWin32Window() const;
	unsigned int GetMemoryType(VkMemoryPropertyFlags memFlags, unsigned int typeFlags) const;

public:
	GLFWwindow* GetWindow() const { return m_Window; }
	VkInstance& GetVulkanInstance() { return m_VulkanInstance; }
	VkPhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }
	VkDevice& GetDevice() { return m_Device; }
	VkQueue& GetQueue() { return m_Queue; }
	VkDescriptorPool& GetDescriptorPool() { return m_DescriptorPool; }
	ImGui_ImplVulkanH_Window* GetImGuiWindow() { return &m_MainWindowData; }
	uint32_t GetQueueFamilyIndex() const { return m_QueueFamily; }
	int GetMinImageCount() const { return m_MinImageCount; }
	bool IsSwapchainRebuild() const { return m_SwapChainRebuild; }

	static CGraphics& GetInstance() { return sm_Instance; }
	static std::stack<std::string>& GetDropFiles() { return sm_DropFiles; }
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

	// Callbacks
	static void DropCallback(GLFWwindow* window, int path_count, const char* paths[]);
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
	static std::stack<std::string> sm_DropFiles;
};

#endif // UI_BACKEND

#endif // !_GRAPHICS_H_