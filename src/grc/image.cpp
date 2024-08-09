//
//	grc/image.cpp
//

#ifdef UI_BACKEND

// Project
#include "image.h"
#include "main/main.h"

// vendor
#include <stb_image.h>

CImage::CImage(unsigned int width, unsigned int height, const void* data) :
	m_Width(width),
	m_Height(height),
	m_AlignedSize(0),
	m_Image(VK_NULL_HANDLE),
	m_ImageView(VK_NULL_HANDLE),
	m_Memory(VK_NULL_HANDLE),
	m_Sampler(VK_NULL_HANDLE),
	m_StagingBuffer(VK_NULL_HANDLE),
	m_StagingBufferMemory(VK_NULL_HANDLE),
	m_DescriptorSet(VK_NULL_HANDLE)
{
	AllocateMemory();
	if (data)
	{
		LoadImageData(data);
	}
}

CImage::~CImage()
{
	Release();
}

CImage* CImage::FromMemory(const void* pMemory, size_t length)
{
	int width, height, comp;
	stbi_uc* data = stbi_load_from_memory((const stbi_uc*)pMemory, (int)length, &width, &height, &comp, 4);
	STBI_ASSERT(data);
	CImage* pImage = GXT_NEW CImage(width, height, data);
	STBI_FREE(data);
	return pImage;
}

void CImage::AllocateMemory()
{
	CGraphics& grc = CGraphics::GetInstance();
	{
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageCreateInfo.extent.width = m_Width;
		imageCreateInfo.extent.height = m_Height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ASSERT_VULKAN(vkCreateImage(grc.GetDevice(), &imageCreateInfo, nullptr, &m_Image));

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(grc.GetDevice(), m_Image, &memoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = grc.GetMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryRequirements.memoryTypeBits);
		ASSERT_VULKAN(vkAllocateMemory(grc.GetDevice(), &memoryAllocateInfo, nullptr, &m_Memory));
		ASSERT_VULKAN(vkBindImageMemory(grc.GetDevice(), m_Image, m_Memory, 0));
	}
	{
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = m_Image;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		ASSERT_VULKAN(vkCreateImageView(grc.GetDevice(), &imageViewCreateInfo, nullptr, &m_ImageView));
	}
	{
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.minLod = -1000;
		samplerCreateInfo.maxLod = 1000;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		ASSERT_VULKAN(vkCreateSampler(grc.GetDevice(), &samplerCreateInfo, nullptr, &m_Sampler));
	}
	m_DescriptorSet = ImGui_ImplVulkan_AddTexture(m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void CImage::LoadImageData(const void* data)
{
	CGraphics& grc = CGraphics::GetInstance();

	const size_t imageSize = static_cast<size_t>(m_Width * m_Height * 4 /* VK_FORMAT_R8G8B8A8_UNORM */);

	{
		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = imageSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ASSERT_VULKAN(vkCreateBuffer(grc.GetDevice(), &bufferCreateInfo, nullptr, &m_StagingBuffer));

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(grc.GetDevice(), m_StagingBuffer, &memoryRequirements);
		m_AlignedSize = memoryRequirements.size;

		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = grc.GetMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memoryRequirements.memoryTypeBits);
		ASSERT_VULKAN(vkAllocateMemory(grc.GetDevice(), &memoryAllocateInfo, nullptr, &m_StagingBufferMemory));
		ASSERT_VULKAN(vkBindBufferMemory(grc.GetDevice(), m_StagingBuffer, m_StagingBufferMemory, 0));
	}
	{
		void* pMemoryMap = nullptr;
		ASSERT_VULKAN(vkMapMemory(grc.GetDevice(), m_StagingBufferMemory, 0, m_AlignedSize, 0, &pMemoryMap));
		memcpy(pMemoryMap, data, imageSize);

		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = m_StagingBufferMemory;
		mappedMemoryRange.size = m_AlignedSize;
		ASSERT_VULKAN(vkFlushMappedMemoryRanges(grc.GetDevice(), 1, &mappedMemoryRange));

		vkUnmapMemory(grc.GetDevice(), m_StagingBufferMemory);
	}
	{
		ImGui_ImplVulkanH_Window* wd = grc.GetImGuiWindow();
		ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];

		ASSERT_VULKAN(vkWaitForFences(grc.GetDevice(), 1, &fd->Fence, VK_TRUE, UINT64_MAX));
		ASSERT_VULKAN(vkResetFences(grc.GetDevice(), 1, &fd->Fence));
		ASSERT_VULKAN(vkResetCommandPool(grc.GetDevice(), fd->CommandPool, 0));

		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		ASSERT_VULKAN(vkBeginCommandBuffer(fd->CommandBuffer, &commandBufferBeginInfo));

		VkImageMemoryBarrier copy_barrier = {};
		copy_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		copy_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		copy_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		copy_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		copy_barrier.image = m_Image;
		copy_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_barrier.subresourceRange.levelCount = 1;
		copy_barrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(fd->CommandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &copy_barrier);

		VkBufferImageCopy region = {};
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.layerCount = 1;
		region.imageExtent.width = m_Width;
		region.imageExtent.height = m_Height;
		region.imageExtent.depth = 1;
		vkCmdCopyBufferToImage(fd->CommandBuffer, m_StagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		VkImageMemoryBarrier use_barrier = {};
		use_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		use_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		use_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		use_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		use_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		use_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		use_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		use_barrier.image = m_Image;
		use_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		use_barrier.subresourceRange.levelCount = 1;
		use_barrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(fd->CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &use_barrier);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &fd->CommandBuffer;

		ASSERT_VULKAN(vkEndCommandBuffer(fd->CommandBuffer));
		ASSERT_VULKAN(vkQueueSubmit(grc.GetQueue(), 1, &submitInfo, fd->Fence));
	}
}

void CImage::Release()
{
	VkDevice device = CGraphics::GetInstance().GetDevice();

	ASSERT_VULKAN(vkDeviceWaitIdle(device));

	ImGui_ImplVulkan_RemoveTexture(m_DescriptorSet);
	vkDestroySampler(device, m_Sampler, nullptr);
	vkDestroyImageView(device, m_ImageView, nullptr);
	vkDestroyImage(device, m_Image, nullptr);
	vkFreeMemory(device, m_Memory, nullptr);
	vkDestroyBuffer(device, m_StagingBuffer, nullptr);
	vkFreeMemory(device, m_StagingBufferMemory, nullptr);

	m_Sampler = VK_NULL_HANDLE;
	m_ImageView = VK_NULL_HANDLE;
	m_Image = VK_NULL_HANDLE;
	m_Memory = VK_NULL_HANDLE;
	m_StagingBuffer = VK_NULL_HANDLE;
	m_StagingBufferMemory = VK_NULL_HANDLE;
	m_DescriptorSet = VK_NULL_HANDLE;
}

#endif // UI_BACKEND