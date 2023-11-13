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

// https://stackoverflow.com/questions/565704/how-to-correctly-convert-filesize-in-bytes-into-mega-or-gigabytes
// KB = B >> 10
// MB = KB >> 10 = B >> 20
// GB = MB >> 10 = KB >> 20 = B >> 30



#if _DEBUG
#define TRACK_MEMORY (1)
#else
#define TRACK_MEMORY (0)
#endif // _DEBUG

#ifndef _MAININCLUDE_H_

unsigned g_Allocations = 0;

void* operator new(size_t size, const char* file, int line)
{
	void* pBlock = ::operator new(size);
	#if TRACK_MEMORY
	g_Allocations++;
	printf("[ operator new ] Allocating 0x%llx bytes (%zi KB) @ %p (%s:%i)\n", size, (size >> 10), pBlock, file, line);
	#else
	(void)file;
	(void)line;
	#endif
	return pBlock;
}

void* operator new[](size_t size, const char* file, int line)
{
	void* pBlock = ::operator new[](size);
	#if TRACK_MEMORY
	g_Allocations++;
	printf("[ operator new[] ] Allocating 0x%llx bytes (%zi KB) @ %p (%s:%i)\n", size, (size >> 10), pBlock, file, line);
	#else
	(void)file;
	(void)line;
	#endif
	return pBlock;
}

void operator delete(void* pBlock, const char* file, int line) noexcept
{
	if (pBlock)
	{
		#if TRACK_MEMORY
		g_Allocations--;
		printf("[ operator delete ] Deallocating memory @ %p (%s:%i)\n", pBlock, file, line);
		#else
		(void)file;
		(void)line;
		#endif
		::operator delete(pBlock);
	}
}

void operator delete[](void* pBlock, const char* file, int line) noexcept
{
	if (pBlock)
	{
		#if TRACK_MEMORY
		g_Allocations--;
		printf("[ operator delete[] ] Deallocating memory @ %p (%s:%i)\n", pBlock, file, line);
		#else
		(void)file;
		(void)line;
		#endif
		::operator delete[](pBlock);
	}
}
#else

extern void*	operator new		(size_t size,	const char* file, int line);
extern void*	operator new[]		(size_t size,	const char* file, int line);
extern void		operator delete		(void* pBlock,	const char* file, int line) noexcept;
extern void		operator delete[]	(void* pBlock,	const char* file, int line) noexcept;

#endif

#if TRACK_MEMORY
#define system_new						new(__FILENAME__, __LINE__)
#define system_delete(pBlock)			operator delete(pBlock, __FILENAME__, __LINE__)
#define system_delete_array(pBlock)		operator delete[](pBlock, __FILENAME__, __LINE__)
#else
#define system_new						new(NULL, NULL)
#define system_delete(pBlock)			operator delete(pBlock, NULL, NULL)
#define system_delete_array(pBlock)		operator delete[](pBlock, NULL, NULL)
#endif // TRACK_MEMORY

#endif // _MAIN_H_