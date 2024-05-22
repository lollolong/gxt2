//
//	grc/glfw_vulkan.cpp
//

#ifdef UI_BACKEND

// Project
#include "glfw_vulkan.h"
#include "fonts/fa-solid-900.cpp"
#include "fonts/Roboto-Regular.cpp"
#include "fonts/Nunito-Regular.cpp"

// vendor
#include <IconsFontAwesome6.h>

#include <vector>


#include <cassert>

#define ASSERT_VULKAN(vkRes) assert(vkRes == VK_SUCCESS);

#ifdef _DEBUG
#define VULKAN_DEBUG
#endif

// Data
GLFWwindow*					CGraphics::sm_Window				= nullptr;
VkInstance					CGraphics::sm_VulkanInstance		= VK_NULL_HANDLE;
VkPhysicalDevice			CGraphics::sm_PhysicalDevice		= VK_NULL_HANDLE;
VkDevice					CGraphics::sm_Device				= VK_NULL_HANDLE;
uint32_t					CGraphics::sm_QueueFamily			= (uint32_t)-1;
VkQueue						CGraphics::sm_Queue					= VK_NULL_HANDLE;
VkDebugReportCallbackEXT	CGraphics::sm_DebugReport			= VK_NULL_HANDLE;
VkPipelineCache				CGraphics::sm_PipelineCache			= VK_NULL_HANDLE;
VkDescriptorPool			CGraphics::sm_DescriptorPool		= VK_NULL_HANDLE;

ImGui_ImplVulkanH_Window	CGraphics::sm_MainWindowData;
int							CGraphics::sm_MinImageCount			= 2;
bool						CGraphics::sm_SwapChainRebuild		= false;


bool CGraphics::Init(const string& windowTitle, int width, int height)
{
	if (!glfwInit())
	{
		return false;
	}

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWmonitor* pPrimaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* pVideoMode = glfwGetVideoMode(pPrimaryMonitor);

	int xPos, yPos;
	glfwGetMonitorPos(pPrimaryMonitor, &xPos, &yPos);

	sm_Window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);

	glfwSetWindowPos(sm_Window,
		(xPos + (pVideoMode->width - width)) / 2,
		(yPos + (pVideoMode->height - height)) / 2);

	if (!glfwVulkanSupported())
	{
		printf("GLFW: Vulkan Not Supported\n");
		return false;
	}


	uint32_t extensionsCount = 0;
	vector<const char*> extensions;

	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
	for (uint32_t i = 0; i < extensionsCount; i++)
	{
#ifdef VULKAN_DEBUG
		printf("[glfw][required extension] %s\n", glfwExtensions[i]);
#endif
		extensions.push_back(glfwExtensions[i]);
	}

	CGraphics::InitVulkan(extensions);
	CGraphics::InitVulkanWindow();

	CGraphics::InitImGui();
	return true;
}

void CGraphics::Shutdown()
{
	ASSERT_VULKAN(vkDeviceWaitIdle(CGraphics::sm_Device));

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	CGraphics::CleanupVulkanWindow();
	CGraphics::CleanupVulkan();

	glfwDestroyWindow(CGraphics::sm_Window);
	glfwTerminate();
}

void CGraphics::PreRender()
{
	glfwPollEvents();

	if (CGraphics::sm_SwapChainRebuild)
	{
		int width, height;
		glfwGetFramebufferSize(CGraphics::sm_Window, &width, &height);

		if (width > 0 && height > 0)
		{
			ImGui_ImplVulkan_SetMinImageCount(CGraphics::sm_MinImageCount);
			ImGui_ImplVulkanH_CreateOrResizeWindow(CGraphics::sm_VulkanInstance, CGraphics::sm_PhysicalDevice, CGraphics::sm_Device, &CGraphics::sm_MainWindowData, CGraphics::sm_QueueFamily, nullptr, width, height, CGraphics::sm_MinImageCount);
			CGraphics::sm_MainWindowData.FrameIndex = 0;
			CGraphics::sm_SwapChainRebuild = false;
		}
	}

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void CGraphics::Render()
{
	// Rendering
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
	const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

	ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.00f);
	if (!is_minimized)
	{
		CGraphics::sm_MainWindowData.ClearValue.color.float32[0] = clear_color.x * clear_color.w;
		CGraphics::sm_MainWindowData.ClearValue.color.float32[1] = clear_color.y * clear_color.w;
		CGraphics::sm_MainWindowData.ClearValue.color.float32[2] = clear_color.z * clear_color.w;
		CGraphics::sm_MainWindowData.ClearValue.color.float32[3] = clear_color.w;
		CGraphics::FrameRender(&CGraphics::sm_MainWindowData, draw_data);
		CGraphics::FramePresent(&CGraphics::sm_MainWindowData);
	}

	static bool bShowInit = false;
	if (!bShowInit)
	{
		bShowInit = true;
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		glfwShowWindow(CGraphics::sm_Window);
	}
}

bool CGraphics::IsRunning()
{
	return !glfwWindowShouldClose(CGraphics::sm_Window);
}

void CGraphics::InitPhysicalDevice()
{
	//---------------- Physical Device ----------------
	//
	uint32_t numDevices = 0;
	vector<VkPhysicalDevice> physicalDevices;
	ASSERT_VULKAN(vkEnumeratePhysicalDevices(CGraphics::sm_VulkanInstance, &numDevices, nullptr));
	IM_ASSERT(numDevices > 0);

#ifdef VULKAN_DEBUG
	printf("[vulkan] Found %i GPU(s)\n", numDevices);
#endif

	physicalDevices.resize(numDevices);
	ASSERT_VULKAN(vkEnumeratePhysicalDevices(CGraphics::sm_VulkanInstance, &numDevices, physicalDevices.data()));

	for (VkPhysicalDevice& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties deviceProperties = {};
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			CGraphics::sm_PhysicalDevice = physicalDevice;
			return;
		}
	}

	if (numDevices > 0)
	{
		CGraphics::sm_PhysicalDevice = physicalDevices[0];
	}
}

void CGraphics::InitLogicalDevice()
{
	{
		//---------------- Graphics Queue Family ----------------
		//
		uint32_t numQueueFamilies = 0;
		vector<VkQueueFamilyProperties> queueFamilyProperties;

		vkGetPhysicalDeviceQueueFamilyProperties(CGraphics::sm_PhysicalDevice, &numQueueFamilies, nullptr);
		queueFamilyProperties.resize(numQueueFamilies);
		vkGetPhysicalDeviceQueueFamilyProperties(CGraphics::sm_PhysicalDevice, &numQueueFamilies, queueFamilyProperties.data());

		for (uint32_t i = 0; i < (uint32_t)queueFamilyProperties.size(); i++)
		{
			VkQueueFamilyProperties queueFamilyProps = queueFamilyProperties.at(i);
			if (queueFamilyProps.queueCount > 0 && (queueFamilyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				CGraphics::sm_QueueFamily = i;
				break;
			}
		}
		IM_ASSERT(CGraphics::sm_QueueFamily != (uint32_t)-1);
	}

	{
		//---------------- Logical Device ----------------
		//

		vector<const char*> enabledExtensions;
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
		queueCreateInfo.queueFamilyIndex = CGraphics::sm_QueueFamily;
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

		ASSERT_VULKAN(vkCreateDevice(CGraphics::sm_PhysicalDevice, &deviceCreateInfo, nullptr, &CGraphics::sm_Device));
		vkGetDeviceQueue(CGraphics::sm_Device, CGraphics::sm_QueueFamily, 0, &CGraphics::sm_Queue);
	}
}

void CGraphics::InitDescriptorPool()
{
	//---------------- Descriptor Pool ----------------
	//
	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
	descriptorPoolCreateInfo.pPoolSizes = poolSizes;

	ASSERT_VULKAN(vkCreateDescriptorPool(CGraphics::sm_Device, &descriptorPoolCreateInfo, nullptr, &CGraphics::sm_DescriptorPool));
}

void CGraphics::InitVulkan(vector<const char*>& extensions)
{
	{
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
		vector<VkLayerProperties> layerProperties;
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
		vector<VkExtensionProperties> extensionProperties;
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
		ASSERT_VULKAN(vkCreateInstance(&createInfo, nullptr, &CGraphics::sm_VulkanInstance));
	}

	CGraphics::InitPhysicalDevice();
	CGraphics::InitLogicalDevice();
	CGraphics::InitDescriptorPool();
}

void CGraphics::InitVulkanWindow()
{
	//---------------- Surface ----------------
	//
	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
	ASSERT_VULKAN(glfwCreateWindowSurface(CGraphics::sm_VulkanInstance, CGraphics::sm_Window, nullptr, &vkSurface));


	//---------------- Framebuffers ----------------
	//
	int width, height;
	glfwGetFramebufferSize(CGraphics::sm_Window, &width, &height);


	ImGui_ImplVulkanH_Window* pWindowImpl = &CGraphics::sm_MainWindowData;
	pWindowImpl->Surface = vkSurface;


	//---------------- Window System Integration ----------------
	//
	VkBool32 bWindowSystemIntegration = VK_FALSE;
	ASSERT_VULKAN(vkGetPhysicalDeviceSurfaceSupportKHR(CGraphics::sm_PhysicalDevice, CGraphics::sm_QueueFamily, pWindowImpl->Surface, &bWindowSystemIntegration));

	if (bWindowSystemIntegration != VK_TRUE)
	{
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}


	//---------------- Surface Format ----------------
	//
	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	pWindowImpl->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(CGraphics::sm_PhysicalDevice, pWindowImpl->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);


	//---------------- Select Present Mode ----------------
	//
#ifdef APP_USE_UNLIMITED_FRAME_RATE
	VkPresentModeKHR presentModes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
	VkPresentModeKHR presentModes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
	pWindowImpl->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(CGraphics::sm_PhysicalDevice, pWindowImpl->Surface, &presentModes[0], IM_ARRAYSIZE(presentModes));


	//---------------- SwapChain, RenderPass, Framebuffer ----------------
	//
	ImGui_ImplVulkanH_CreateOrResizeWindow(CGraphics::sm_VulkanInstance, CGraphics::sm_PhysicalDevice, CGraphics::sm_Device, pWindowImpl, CGraphics::sm_QueueFamily, nullptr, width, height, CGraphics::sm_MinImageCount);
}

void CGraphics::CleanupVulkan()
{
	vkDestroyDescriptorPool(CGraphics::sm_Device, CGraphics::sm_DescriptorPool, nullptr);
	vkDestroyDevice(CGraphics::sm_Device, nullptr);
	vkDestroyInstance(CGraphics::sm_VulkanInstance, nullptr);
}

void CGraphics::CleanupVulkanWindow()
{
	ImGui_ImplVulkanH_DestroyWindow(CGraphics::sm_VulkanInstance, CGraphics::sm_Device, &CGraphics::sm_MainWindowData, nullptr);
}

void CGraphics::FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{
	VkResult err;

	VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
	VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
	err = vkAcquireNextImageKHR(CGraphics::sm_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
	{
		CGraphics::sm_SwapChainRebuild = true;
		return;
	}

	ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
	{
		err = vkWaitForFences(CGraphics::sm_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking

		err = vkResetFences(CGraphics::sm_Device, 1, &fd->Fence);
	}
	{
		err = vkResetCommandPool(CGraphics::sm_Device, fd->CommandPool, 0);
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
	}
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = wd->RenderPass;
		info.framebuffer = fd->Framebuffer;
		info.renderArea.extent.width = wd->Width;
		info.renderArea.extent.height = wd->Height;
		info.clearValueCount = 1;
		info.pClearValues = &wd->ClearValue;
		vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

	// Submit command buffer
	vkCmdEndRenderPass(fd->CommandBuffer);
	{
		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &image_acquired_semaphore;
		info.pWaitDstStageMask = &wait_stage;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &fd->CommandBuffer;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &render_complete_semaphore;

		err = vkEndCommandBuffer(fd->CommandBuffer);
		err = vkQueueSubmit(CGraphics::sm_Queue, 1, &info, fd->Fence);
	}
}

void CGraphics::FramePresent(ImGui_ImplVulkanH_Window* wd)
{
	if (CGraphics::sm_SwapChainRebuild)
		return;
	VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &render_complete_semaphore;
	info.swapchainCount = 1;
	info.pSwapchains = &wd->Swapchain;
	info.pImageIndices = &wd->FrameIndex;
	VkResult err = vkQueuePresentKHR(CGraphics::sm_Queue, &info);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
	{
		CGraphics::sm_SwapChainRebuild = true;
		return;
	}
	wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount; // Now we can use the next set of semaphores
}

void CGraphics::InitImGui()
{
	//---------------- Dear ImGui Context ----------------
	//
	IM_ASSERT(IMGUI_CHECKVERSION());
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;


	//---------------- Platform & Renderer Backends ----------------
	//
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = CGraphics::sm_VulkanInstance;
	initInfo.PhysicalDevice = CGraphics::sm_PhysicalDevice;
	initInfo.Device = CGraphics::sm_Device;
	initInfo.QueueFamily = CGraphics::sm_QueueFamily;
	initInfo.Queue = CGraphics::sm_Queue;
	initInfo.PipelineCache = CGraphics::sm_PipelineCache;
	initInfo.DescriptorPool = CGraphics::sm_DescriptorPool;
	initInfo.RenderPass = CGraphics::sm_MainWindowData.RenderPass;
	initInfo.Subpass = 0;
	initInfo.MinImageCount = CGraphics::sm_MinImageCount;
	initInfo.ImageCount = CGraphics::sm_MainWindowData.ImageCount;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplGlfw_InitForVulkan(CGraphics::sm_Window, false);
	ImGui_ImplVulkan_Init(&initInfo);


	//---------------- Fonts & Theme ----------------
	//
	CGraphics::SetupFonts();
	CGraphics::SetupTheme();
}

void CGraphics::SetupFonts()
{
	ImGuiIO& io = ImGui::GetIO();

	// Roboto
	ImFontConfig robotoConfig;
	robotoConfig.FontDataOwnedByAtlas = false;

	ImFont* pRobotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_FontRobotoRegular, sizeof(g_FontRobotoRegular), 16.f, &robotoConfig);
	io.FontDefault = pRobotoFont;
	IM_ASSERT(pRobotoFont != nullptr);


	// Font Awesome
	float baseFontSize = 13.0f; // 13.0f is the size of the default font. Change to the font size you use.
	float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

	// merge in icons from Font Awesome
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = iconFontSize;
	icons_config.FontDataOwnedByAtlas = false;
	ImFont* pFontAwesome = io.Fonts->AddFontFromMemoryTTF((void*)g_FontAwesomeSolid900, sizeof(g_FontAwesomeSolid900), 16.f, &icons_config, icons_ranges);
	IM_ASSERT(pFontAwesome != nullptr);
	(void)pFontAwesome;
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