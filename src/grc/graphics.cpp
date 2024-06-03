//
//	grc/graphics.cpp
//

#ifdef UI_BACKEND

// Project
#include "graphics.h"
#include "main/gxt2edit.h"
#include "resources/resource.h"
#include "fonts/fa-solid-900.cpp"
#include "fonts/NotoSans-Regular.cpp"

// Windows
#ifdef _WIN32
	#include <dwmapi.h>
	#define DARK_MODE_STRING_NAME	L"DarkMode_Explorer"
	#define DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 19
#endif

// vendor
#include <IconsFontAwesome6.h>

// C/C++
#include <cstdio>

CGraphics CGraphics::sm_Instance;
std::stack<std::string> CGraphics::sm_DropFiles;
bool CGraphics::sm_InClosingState;

CGraphics::CGraphics() :
	m_Window(nullptr),
	m_Width(0),
	m_Height(0),
	m_VulkanInstance(VK_NULL_HANDLE),
	m_PhysicalDevice(VK_NULL_HANDLE),
	m_Device(VK_NULL_HANDLE),
	m_QueueFamily((uint32_t)-1),
	m_Queue(VK_NULL_HANDLE),
	m_DescriptorPool(VK_NULL_HANDLE),
	m_MinImageCount(2),
	m_SwapChainRebuild(false)
{
}

bool CGraphics::Init(const std::string& windowTitle, int width, int height)
{
	glfwSetErrorCallback([](int error, const char* description) -> void
	{
		printf("GLFW: Error %i (%s)\n", error, description);
	});

	if (!glfwInit())
	{
		return false;
	}

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_Window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);

	// Linux issue:
	//  Wayland does not support window positioning
	//  You could argue x11 does, but x11 is genuinely deprecated and old
	// 	Most people these days use Wayland
#if _WIN32
	GLFWmonitor* pPrimaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* pVideoMode = glfwGetVideoMode(pPrimaryMonitor);

	int xPos, yPos;
	glfwGetMonitorPos(pPrimaryMonitor, &xPos, &yPos);

	glfwSetWindowPos(m_Window,
		(xPos + (pVideoMode->width - width)) / 2,
		(yPos + (pVideoMode->height - height)) / 2);
#endif

	glfwSetDropCallback(m_Window, DropCallback);
	glfwSetWindowCloseCallback(m_Window, CloseCallback);

	if (!glfwVulkanSupported())
	{
		printf("GLFW: Vulkan Not Supported\n");
		return false;
	}

#ifdef _WIN32
	SetClassLongPtr(GetWin32Window(), GCLP_HICON, (LONG_PTR)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON)));

	const BOOL isDarkMode = TRUE;

	//SetWindowTheme(GetWin32Window(), DARK_MODE_STRING_NAME, nullptr);
	if (FAILED(DwmSetWindowAttribute(GetWin32Window(), DWMWA_USE_IMMERSIVE_DARK_MODE, &isDarkMode, sizeof(isDarkMode))))
	{
		DwmSetWindowAttribute(GetWin32Window(), DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &isDarkMode, sizeof(isDarkMode));
	}
#endif

#if linux
	// Window has to be visible on surface creation for vulkan
	// https://github.com/glfw/glfw/issues/1268#issuecomment-392246281
	glfwShowWindow(m_Window);
#endif

	glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) -> void
	{
#if _DEBUG
		printf("[glfwSetWindowSizeCallback] window = %p\n", window);
		printf("[glfwSetWindowSizeCallback] width = %i, height = %i\n", width, height);
#endif

		if (width > 0 && height > 0)
		{
			// Render
			CGraphics::GetInstance().PreRender();
			gxt2edit::GetInstance().Draw();
			CGraphics::GetInstance().Render();
		}

		// Params
		IM_UNUSED(window);
	});

	CGraphics::InitVulkan();
	CGraphics::InitImGui();
	return true;
}

void CGraphics::Shutdown()
{
	ASSERT_VULKAN(vkDeviceWaitIdle(m_Device));

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	CGraphics::CleanupVulkanWindow();
	CGraphics::CleanupVulkan();

	glfwDestroyWindow(m_Window);
	glfwTerminate();

	m_Window = nullptr;
	m_Queue = VK_NULL_HANDLE;
	m_PhysicalDevice = VK_NULL_HANDLE;
	m_QueueFamily = (uint32_t)-1;
}

void CGraphics::PreRender()
{
	int width, height;
	glfwGetFramebufferSize(m_Window, &width, &height);

	if (width != m_Width || height != m_Height)
	{
		m_Width = width;
		m_Height = height;
		m_SwapChainRebuild = true;
	}

	if (m_SwapChainRebuild)
	{
		if (width > 0 && height > 0)
		{
			ImGui_ImplVulkan_SetMinImageCount(m_MinImageCount);
			ImGui_ImplVulkanH_CreateOrResizeWindow(m_VulkanInstance, m_PhysicalDevice, m_Device, &m_MainWindowData, m_QueueFamily, nullptr, width, height, m_MinImageCount);
			m_MainWindowData.FrameIndex = 0;
			m_SwapChainRebuild = false;
		}
	}

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void CGraphics::Render()
{
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	const bool isMinimized = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);

	ImVec4 clearColor = ImVec4(0.f, 0.f, 0.f, 1.00f);
	if (!isMinimized)
	{
		m_MainWindowData.ClearValue.color.float32[0] = clearColor.x * clearColor.w;
		m_MainWindowData.ClearValue.color.float32[1] = clearColor.y * clearColor.w;
		m_MainWindowData.ClearValue.color.float32[2] = clearColor.z * clearColor.w;
		m_MainWindowData.ClearValue.color.float32[3] = clearColor.w;
		FrameRender(&m_MainWindowData, drawData);
		FramePresent(&m_MainWindowData);
	}

	//---------------- Avoid Blank Screen ----------------
	//-------------------- on Startup --------------------
	//
	static bool bMakeWindowVisible = false;
	if (!bMakeWindowVisible)
	{
		bMakeWindowVisible = true;
		glfwShowWindow(m_Window);
	}
}

bool CGraphics::IsRunning() const
{
	return !glfwWindowShouldClose(m_Window);
}

bool CGraphics::IsMinimized() const
{
	return glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED);
}

void CGraphics::CloseWindow(bool bClose) const
{
#if _DEBUG
	printf("[%s] Closing State = %i\n", __FUNCTION__, bClose);
#endif

	if (bClose)
	{
		glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
	}
	else
	{
		glfwSetWindowShouldClose(m_Window, GLFW_FALSE);
	}
}

void CGraphics::PollEvents() const
{
	glfwPollEvents();
}

#ifdef _WIN32
HWND CGraphics::GetWin32Window() const
{
	return glfwGetWin32Window(m_Window);
}
#endif

unsigned int CGraphics::GetMemoryType(VkMemoryPropertyFlags memFlags, unsigned int typeFlags) const
{
	VkPhysicalDeviceMemoryProperties properties;
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &properties);
	for (unsigned int iType = 0; iType < properties.memoryTypeCount; iType++)
	{
		if ((properties.memoryTypes[iType].propertyFlags & memFlags) == memFlags && typeFlags & (1 << iType))
			return iType;
	}
	return 0XFFFFFFFF;
}

void CGraphics::DropCallback(GLFWwindow* window, int path_count, const char* paths[])
{
#if _DEBUG
	printf("[%s] window = %p\n", __FUNCTION__, window);
	for (int i = 0; i < path_count; i++)
	{
		printf("[%s] paths[%i] = %s\n", __FUNCTION__, i, paths[i]);
	}
#endif

	if (path_count > 0)
	{
		sm_DropFiles.push(paths[0]);
	}

	IM_UNUSED(window);
}

void CGraphics::CloseCallback(GLFWwindow* window)
{
#if _DEBUG
	printf("[%s] Settings Closing State!\n", __FUNCTION__);
#endif

	SetIsClosing(true);
	IM_UNUSED(window);
}

//---------------- Init ----------------
//

void CGraphics::InitVulkan()
{
	{
		//---------------- Required Instance Extensions ----------------
		//
		uint32_t extensionsCount = 0;
		std::vector<const char*> extensions;

		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
		for (uint32_t i = 0; i < extensionsCount; i++)
		{
#ifdef VULKAN_DEBUG
			printf("[glfw][required extension] %s\n", glfwExtensions[i]);
#endif
			extensions.push_back(glfwExtensions[i]);
		}

		//---------------- Application Info ----------------
		//
		VkApplicationInfo applicationInfo = {};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = "gxt2edit";
		applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.apiVersion = VK_API_VERSION_1_3;

		//---------------- Create Info ----------------
		//
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &applicationInfo;

#ifdef VULKAN_DEBUG
		//---------------- Instance Layer Properties ----------------
		//
		uint32_t layerPropertyCount = 0;
		std::vector<VkLayerProperties> layerProperties;
		ASSERT_VULKAN(vkEnumerateInstanceLayerProperties(&layerPropertyCount, nullptr));

		layerProperties.resize(layerPropertyCount);
		ASSERT_VULKAN(vkEnumerateInstanceLayerProperties(&layerPropertyCount, layerProperties.data()));

		for (const VkLayerProperties& vkLayerProperty : layerProperties)
		{
			printf("[vulkan][layer property] %s (%s)\n", vkLayerProperty.layerName, vkLayerProperty.description);
		}
#endif

		//---------------- Instance Extension Properties ----------------
		//
		uint32_t instanceExtensionCount = 0;
		std::vector<VkExtensionProperties> extensionProperties;
		ASSERT_VULKAN(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr));

		extensionProperties.resize(instanceExtensionCount);
		ASSERT_VULKAN(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, extensionProperties.data()));

		for (const VkExtensionProperties& vkExtensionProperty : extensionProperties)
		{
			if (strcmp(vkExtensionProperty.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
			{
				extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
			}

#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
			if (strcmp(vkExtensionProperty.extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0)
			{
				extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
				createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
			}
#endif

#ifdef VULKAN_DEBUG
			printf("[vulkan][extension property] %s\n", vkExtensionProperty.extensionName);
#endif
		}

#ifdef VULKAN_DEBUG
		//---------------- Validation layers ----------------
		//
		const char* enabledLayers[] = {
			"VK_LAYER_KHRONOS_validation"
		};
		createInfo.enabledLayerCount = (uint32_t)std::size(enabledLayers);
		createInfo.ppEnabledLayerNames = enabledLayers;
		extensions.push_back("VK_EXT_debug_report");
#endif

		//---------------- Create Vulkan Instance ----------------
		//
		createInfo.enabledExtensionCount = (uint32_t)extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();
		ASSERT_VULKAN(vkCreateInstance(&createInfo, nullptr, &m_VulkanInstance));
	}

	CGraphics::InitPhysicalDevice();
	CGraphics::InitLogicalDevice();
	CGraphics::InitDescriptorPool();

	CGraphics::InitVulkanWindow();
}

void CGraphics::InitVulkanWindow()
{
	//---------------- Surface ----------------
	//
	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
	ASSERT_VULKAN(glfwCreateWindowSurface(m_VulkanInstance, m_Window, nullptr, &vkSurface));

	//---------------- Framebuffers ----------------
	//
	int width, height;
	glfwGetFramebufferSize(m_Window, &width, &height);

	ImGui_ImplVulkanH_Window* pWindowImpl = &m_MainWindowData;
	pWindowImpl->Surface = vkSurface;

	//---------------- Window System Integration ----------------
	//
	VkBool32 bWindowSystemIntegration = VK_FALSE;
	ASSERT_VULKAN(vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, m_QueueFamily, pWindowImpl->Surface, &bWindowSystemIntegration));

	if (bWindowSystemIntegration != VK_TRUE)
	{
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}

	//---------------- Surface Format ----------------
	//
	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	pWindowImpl->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(m_PhysicalDevice, pWindowImpl->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

	//---------------- Select Present Mode ----------------
	//
#ifdef APP_USE_UNLIMITED_FRAME_RATE
	VkPresentModeKHR presentModes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
	VkPresentModeKHR presentModes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
	pWindowImpl->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(m_PhysicalDevice, pWindowImpl->Surface, &presentModes[0], IM_ARRAYSIZE(presentModes));

	//---------------- SwapChain, RenderPass, Framebuffer ----------------
	//
	ImGui_ImplVulkanH_CreateOrResizeWindow(m_VulkanInstance, m_PhysicalDevice, m_Device, pWindowImpl, m_QueueFamily, nullptr, width, height, m_MinImageCount);
}

void CGraphics::InitPhysicalDevice()
{
	//---------------- Physical Device ----------------
	//
	uint32_t numDevices = 0;
	std::vector<VkPhysicalDevice> physicalDevices;
	ASSERT_VULKAN(vkEnumeratePhysicalDevices(m_VulkanInstance, &numDevices, nullptr));
	IM_ASSERT(numDevices > 0);

#ifdef VULKAN_DEBUG
	printf("[vulkan] Found %i GPU(s)\n", numDevices);
#endif

	physicalDevices.resize(numDevices);
	ASSERT_VULKAN(vkEnumeratePhysicalDevices(m_VulkanInstance, &numDevices, physicalDevices.data()));

	for (VkPhysicalDevice& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties deviceProperties = {};
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			m_PhysicalDevice = physicalDevice;
			return;
		}
	}

	if (numDevices > 0)
	{
		m_PhysicalDevice = physicalDevices[0];
	}
}

void CGraphics::InitLogicalDevice()
{
	{
		//---------------- Graphics Queue Family ----------------
		//
		uint32_t numQueueFamilies = 0;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;

		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &numQueueFamilies, nullptr);
		queueFamilyProperties.resize(numQueueFamilies);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &numQueueFamilies, queueFamilyProperties.data());

		for (uint32_t i = 0; i < (uint32_t)queueFamilyProperties.size(); i++)
		{
			VkQueueFamilyProperties queueFamilyProps = queueFamilyProperties.at(i);
			if (queueFamilyProps.queueCount > 0 && (queueFamilyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				m_QueueFamily = i;
				break;
			}
		}
		IM_ASSERT(m_QueueFamily != (uint32_t)-1);
	}

	{
		//---------------- Logical Device ----------------
		//

		std::vector<const char*> enabledExtensions;
		enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
		uint32_t extensionPropertiesCount = 0;
		vector<VkExtensionProperties> extensionProperties;
		ASSERT_VULKAN(vkEnumerateDeviceExtensionProperties(CGraphics::sm_PhysicalDevice, nullptr, &extensionPropertiesCount, nullptr));

		extensionProperties.resize(extensionPropertiesCount);
		ASSERT_VULKAN(vkEnumerateDeviceExtensionProperties(CGraphics::sm_PhysicalDevice, nullptr, &extensionPropertiesCount, extensionProperties.data()));

		for (const VkExtensionProperties extensionProperty : extensionProperties)
		{
			if (strcmp(extensionProperty.extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) == 0)
			{
				enabledExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
			}
		}
#endif

		//---------------- Device Queue Create Info ----------------
		//
		const float queuePriorities[] = { 1.0f };

		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = m_QueueFamily;
		queueCreateInfo.queueCount = (uint32_t)std::size(queuePriorities);
		queueCreateInfo.pQueuePriorities = queuePriorities;

		//---------------- Device Create Info ----------------
		//
		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

		ASSERT_VULKAN(vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device));
		vkGetDeviceQueue(m_Device, m_QueueFamily, 0, &m_Queue);
	}
}

void CGraphics::InitDescriptorPool()
{
	//---------------- Descriptor Pool ----------------
	//
	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
	descriptorPoolCreateInfo.pPoolSizes = poolSizes;

	ASSERT_VULKAN(vkCreateDescriptorPool(m_Device, &descriptorPoolCreateInfo, nullptr, &m_DescriptorPool));
}

//---------------- Cleanup ----------------
//

void CGraphics::CleanupVulkan()
{
	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
	vkDestroyDevice(m_Device, nullptr);
	vkDestroyInstance(m_VulkanInstance, nullptr);

	m_Device = VK_NULL_HANDLE;
	m_DescriptorPool = VK_NULL_HANDLE;
	m_VulkanInstance = VK_NULL_HANDLE;
}

void CGraphics::CleanupVulkanWindow()
{
	ImGui_ImplVulkanH_DestroyWindow(m_VulkanInstance, m_Device, &m_MainWindowData, nullptr);
}

//---------------- Render ----------------
//

void CGraphics::FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* drawData)
{
	VkSemaphore imageAcquiredSemaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
	VkSemaphore renderCompleteSemaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;

	VkResult err = vkAcquireNextImageKHR(m_Device, wd->Swapchain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &wd->FrameIndex);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
	{
		m_SwapChainRebuild = true;
		return;
	}

	ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];

	ASSERT_VULKAN(vkWaitForFences(m_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX));
	ASSERT_VULKAN(vkResetFences(m_Device, 1, &fd->Fence));
	ASSERT_VULKAN(vkResetCommandPool(m_Device, fd->CommandPool, 0));

	//---------------- Command Buffer ----------------
	//
	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	ASSERT_VULKAN(vkBeginCommandBuffer(fd->CommandBuffer, &commandBufferBeginInfo));

	//---------------- Render Pass ----------------
	//
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = wd->RenderPass;
	renderPassBeginInfo.framebuffer = fd->Framebuffer;
	renderPassBeginInfo.renderArea.extent.width = wd->Width;
	renderPassBeginInfo.renderArea.extent.height = wd->Height;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &wd->ClearValue;
	vkCmdBeginRenderPass(fd->CommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	ImGui_ImplVulkan_RenderDrawData(drawData, fd->CommandBuffer);
	vkCmdEndRenderPass(fd->CommandBuffer);

	//---------------- Queue Submit ----------------
	//
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAcquiredSemaphore;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &fd->CommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderCompleteSemaphore;

	ASSERT_VULKAN(vkEndCommandBuffer(fd->CommandBuffer));
	ASSERT_VULKAN(vkQueueSubmit(m_Queue, 1, &submitInfo, fd->Fence));
}

void CGraphics::FramePresent(ImGui_ImplVulkanH_Window* wd)
{
	if (m_SwapChainRebuild)
	{
		return;
	}

	VkSemaphore vkSemaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &vkSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &wd->Swapchain;
	presentInfo.pImageIndices = &wd->FrameIndex;

	VkResult err = vkQueuePresentKHR(m_Queue, &presentInfo);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
	{
		m_SwapChainRebuild = true;
		return;
	}
	wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount;
}

//---------------- ImGui ----------------
//

void CGraphics::InitImGui()
{
	//---------------- Dear ImGui Context ----------------
	//
	IM_ASSERT(IMGUI_CHECKVERSION());
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	//---------------- Platform & Renderer Backends ----------------
	//
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = m_VulkanInstance;
	initInfo.PhysicalDevice = m_PhysicalDevice;
	initInfo.Device = m_Device;
	initInfo.QueueFamily = m_QueueFamily;
	initInfo.Queue = m_Queue;
	initInfo.DescriptorPool = m_DescriptorPool;
	initInfo.RenderPass = m_MainWindowData.RenderPass;
	initInfo.Subpass = 0;
	initInfo.MinImageCount = m_MinImageCount;
	initInfo.ImageCount = m_MainWindowData.ImageCount;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
#ifdef VULKAN_DEBUG
	initInfo.CheckVkResultFn = CheckVulkan;
#endif

	ImGui_ImplGlfw_InitForVulkan(m_Window, true);
	ImGui_ImplVulkan_Init(&initInfo);

	//---------------- Fonts & Theme ----------------
	//
	CGraphics::SetupFonts();
	CGraphics::SetupTheme();
}

void CGraphics::SetupFonts()
{
	ImGuiIO& io = ImGui::GetIO();

	//---------------- Noto ----------------
	//
	ImFontConfig notoConfig;
	notoConfig.FontDataOwnedByAtlas = false;

	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0100, 0x017F, // Latin
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0xA640, 0xA69F, // Cyrillic Extended-B
		0,
	};

	io.Fonts->AddFontFromMemoryTTF((void*)g_FontNotoRegular, sizeof(g_FontNotoRegular), 18.f, &notoConfig, ranges /* io.Fonts->GetGlyphRangesCyrillic() */);

	//---------------- Noto CJK ----------------
	//
	ImFontConfig notoConfigCJK;
	notoConfigCJK.MergeMode = true;
	notoConfigCJK.PixelSnapH = true;

#if _WIN32
	char szModulePath[MAX_PATH] = {};
	GetModuleFileNameA(NULL, szModulePath, sizeof(szModulePath));

	std::string modulePath = szModulePath;
	modulePath = modulePath.substr(0, modulePath.find_last_of('\\'));

	const std::filesystem::path fontsPath = std::filesystem::path(modulePath) / "fonts";
	if (std::filesystem::exists(fontsPath))
	{
		io.Fonts->AddFontFromFileTTF((fontsPath / "NotoSansJP-Regular.ttf").string().c_str(), 18.f, &notoConfigCJK, io.Fonts->GetGlyphRangesJapanese());
		io.Fonts->AddFontFromFileTTF((fontsPath / "NotoSansKR-Regular.ttf").string().c_str(), 18.f, &notoConfigCJK, io.Fonts->GetGlyphRangesKorean());
		io.Fonts->AddFontFromFileTTF((fontsPath / "NotoSansSC-Regular.ttf").string().c_str(), 18.f, &notoConfigCJK, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
		io.Fonts->AddFontFromFileTTF((fontsPath / "NotoSansTC-Regular.ttf").string().c_str(), 18.f, &notoConfigCJK, io.Fonts->GetGlyphRangesChineseFull());
	}
#else
	std::filesystem::path baseFontPath = std::filesystem::current_path();

	if (IS_APPIMAGE_BUILD)
	{
		baseFontPath = std::getenv("APPDIR");
	}

	io.Fonts->AddFontFromFileTTF((baseFontPath / "fonts/NotoSansJP-Regular.ttf").string().c_str(), 18.f, &notoConfigCJK, io.Fonts->GetGlyphRangesJapanese());
	io.Fonts->AddFontFromFileTTF((baseFontPath / "fonts/NotoSansKR-Regular.ttf").string().c_str(), 18.f, &notoConfigCJK, io.Fonts->GetGlyphRangesKorean());
	io.Fonts->AddFontFromFileTTF((baseFontPath / "fonts/NotoSansSC-Regular.ttf").string().c_str(), 18.f, &notoConfigCJK, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
	io.Fonts->AddFontFromFileTTF((baseFontPath / "fonts/NotoSansTC-Regular.ttf").string().c_str(), 18.f, &notoConfigCJK, io.Fonts->GetGlyphRangesChineseFull());
#endif

	//---------------- Font Awesome ----------------
	//
	const float baseFontSize = 13.0f; // 13.0f is the size of the default font. Change to the font size you use.
	const float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

	ImFontConfig iconsConfig;
	iconsConfig.MergeMode = true;
	iconsConfig.PixelSnapH = true;
	iconsConfig.GlyphMinAdvanceX = iconFontSize;
	iconsConfig.FontDataOwnedByAtlas = false;

	static const ImWchar rangesFA[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	io.Fonts->AddFontFromMemoryTTF((void*)g_FontAwesomeSolid900, sizeof(g_FontAwesomeSolid900), 16.f, &iconsConfig, rangesFA);

	//---------------- Build Atlas ----------------
	//
	io.Fonts->Build();
}

void CGraphics::SetupTheme()
{
	// Future Dark style by rewrking from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 1.0f;
	style.WindowPadding = ImVec2(12.0f, 12.0f);
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(20.0f, 20.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(6.0f, 6.0f);
	style.FrameRounding = 0.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(12.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(6.0f, 3.0f);
	style.CellPadding = ImVec2(12.0f, 6.0f);
	style.IndentSpacing = 20.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 12.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabMinSize = 12.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.TabBorderSize = 0.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5372549295425415f, 0.5529412031173706f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.2901960909366608f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.9960784316062927f, 0.4745098054409027f, 0.6980392336845398f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
}

#endif // UI_BACKEND