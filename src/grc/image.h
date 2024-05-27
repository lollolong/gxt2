//
//	grc/image.h
//

#ifndef _IMAGE_H_
#define _IMAGE_H_

#ifdef UI_BACKEND

// Project
#include "graphics.h"

class CImage
{
public:
	CImage(unsigned int width, unsigned int height, const void* data);
	virtual ~CImage();

	void LoadImageData(const void* data);

	unsigned int GetWidth() const { return m_Width; }
	unsigned int GetHeight() const { return m_Height; }

	VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }
	ImTextureID GetTextureId() const { return static_cast<ImTextureID>(GetDescriptorSet()); }

	template<typename T, std::size_t N>
	static CImage* FromMemory(T(&pMemory)[N])
	{
		return FromMemory(pMemory, N);
	}
	static CImage* FromMemory(const void* pMemory, size_t length);

private:
	void AllocateMemory();
	void Release();

private:
	unsigned int m_Width;
	unsigned int m_Height;
	size_t m_AlignedSize;

	VkImage m_Image;
	VkImageView m_ImageView;
	VkDeviceMemory m_Memory;
	VkSampler m_Sampler;
	VkBuffer m_StagingBuffer;
	VkDeviceMemory m_StagingBufferMemory;
	VkDescriptorSet m_DescriptorSet;
};

#endif

#endif