//
//	main/main.h
//

#ifndef _MAIN_H_
#define _MAIN_H_

// C/C++
#include <cstdio>

// https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
#ifdef _MSC_VER
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif // _MSC_VER

#if _WIN32
#if _DEBUG
#define TRACK_MEMORY (1)
#else
#define TRACK_MEMORY (0)
#endif // _DEBUG

// https://stackoverflow.com/questions/565704/how-to-correctly-convert-filesize-in-bytes-into-mega-or-gigabytes
// KB = B >> 10
// MB = KB >> 10 = B >> 20
// GB = MB >> 10 = KB >> 20 = B >> 30

inline void* operator new(size_t size, const char* file, int line)
{
	void* pBlock = ::operator new(size);
#if TRACK_MEMORY
	printf("[operator new] Allocating 0x%llx bytes (%zi KB) @ %p (%s:%i)\n", size, (size >> 10), pBlock, file, line);
#else
	(void)file;
	(void)line;
#endif
	return pBlock;
}

inline void* operator new[](size_t size, const char* file, int line)
{
	void* pBlock = ::operator new[](size);
#if TRACK_MEMORY
	printf("[operator new[]] Allocating 0x%llx bytes (%zi KB) @ %p (%s:%i)\n", size, (size >> 10), pBlock, file, line);
#else
	(void)file;
	(void)line;
#endif
	return pBlock;
}

inline void operator delete(void* pBlock, const char* file, int line) noexcept
{
	if (pBlock)
	{
#if TRACK_MEMORY
		printf("[operator delete] Deallocating memory @ %p (%s:%i)\n", pBlock, file, line);
#else
		(void)file;
		(void)line;
#endif
		::operator delete(pBlock);
	}
}

inline void operator delete[](void* pBlock, const char* file, int line) noexcept
{
	if (pBlock)
	{
#if TRACK_MEMORY
		printf("[operator delete[]] Deallocating memory @ %p (%s:%i)\n", pBlock, file, line);
#else
		(void)file;
		(void)line;
#endif
		::operator delete[](pBlock);
	}
}

#if TRACK_MEMORY
#define GXT_NEW						new(__FILENAME__, __LINE__)
#define GXT_FREE(pBlock)			operator delete(pBlock, __FILENAME__, __LINE__)
#define GXT_FREE_ARRAY(pBlock)		operator delete[](pBlock, __FILENAME__, __LINE__)
#else
#define GXT_NEW						new(NULL, 0)
#define GXT_FREE(pBlock)			operator delete(pBlock, NULL, NULL)
#define GXT_FREE_ARRAY(pBlock)		operator delete[](pBlock, NULL, NULL)
#endif // TRACK_MEMORY

#else
#define GXT_NEW						new 
#define GXT_FREE(pBlock)			delete pBlock
#define GXT_FREE_ARRAY(pBlock)		delete[] pBlock
#endif

#endif // !_MAIN_H_