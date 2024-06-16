//
//	main/main.h
//

#ifndef _MAIN_H_
#define _MAIN_H_

// C/C++
#include <chrono>
#include <cstdio>


//---------------- Config Switches ----------------
//
#if _DEBUG
	#define TRACK_MEMORY		(1)
	#define MEASURE_ENABLED		(0)
#else
	#define TRACK_MEMORY		(0)
	#define MEASURE_ENABLED		(0)
#endif // _DEBUG


//---------------- Filename ----------------
//
// 
// https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
#ifdef _MSC_VER
	#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
	#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif // _MSC_VER


//---------------- Measure ----------------
//
#if MEASURE_ENABLED
	#define _TIMEPOINT_T	const std::chrono::steady_clock::time_point
	#define _TIMEPOINT_NOW	std::chrono::high_resolution_clock::now()

	#define MEASURE_START	_TIMEPOINT_T startpoint = _TIMEPOINT_NOW
	#define MEASURE_END		_TIMEPOINT_T endpoint = _TIMEPOINT_NOW

	#define MEASURE_NANO	std::chrono::duration_cast<std::chrono::nanoseconds>(endpoint - startpoint).count()
	#define MEASURE_MICRO	std::chrono::duration_cast<std::chrono::microseconds>(endpoint - startpoint).count()
	#define MEASURE_MS		std::chrono::duration_cast<std::chrono::milliseconds>(endpoint - startpoint).count()
#else
	#define MEASURE_START
	#define MEASURE_END

	#define MEASURE_NANO	0LL
	#define MEASURE_MICRO	0LL
	#define MEASURE_MS		0LL
#endif // MEASURE_ENABLED


//---------------- Memory ----------------
//
#if _WIN32 && TRACK_MEMORY

// https://stackoverflow.com/questions/565704/how-to-correctly-convert-filesize-in-bytes-into-mega-or-gigabytes
// KB = B >> 10
// MB = KB >> 10 = B >> 20
// GB = MB >> 10 = KB >> 20 = B >> 30

inline void* operator new(size_t size, const char* file, int line)
{
	void* pBlock = ::operator new(size);
	printf("[operator new] Allocating 0x%llx bytes (%zi KB) @ %p (%s:%i)\n", size, (size >> 10), pBlock, file, line);
	return pBlock;
}

inline void* operator new[](size_t size, const char* file, int line)
{
	void* pBlock = ::operator new[](size);
	printf("[operator new[]] Allocating 0x%llx bytes (%zi KB) @ %p (%s:%i)\n", size, (size >> 10), pBlock, file, line);
	return pBlock;
}

inline void operator delete(void* pBlock, const char* file, int line) noexcept
{
	if (pBlock)
	{
		printf("[operator delete] Deallocating memory @ %p (%s:%i)\n", pBlock, file, line);
		::operator delete(pBlock);
	}
}

inline void operator delete[](void* pBlock, const char* file, int line) noexcept
{
	if (pBlock)
	{
		printf("[operator delete[]] Deallocating memory @ %p (%s:%i)\n", pBlock, file, line);
		::operator delete[](pBlock);
	}
}
#endif // _WIN32 && TRACK_MEMORY

#if _WIN32 && TRACK_MEMORY
	#define GXT_NEW						new(__FILENAME__, __LINE__)
	#define GXT_FREE(pBlock)			operator delete(pBlock, __FILENAME__, __LINE__)
	#define GXT_FREE_ARRAY(pBlock)		operator delete[](pBlock, __FILENAME__, __LINE__)
#else
	#define GXT_NEW						new
	#define GXT_FREE(pBlock)			delete pBlock
	#define GXT_FREE_ARRAY(pBlock)		delete[] pBlock
#endif // _WIN32 && TRACK_MEMORY

#endif // !_MAIN_H_