#pragma once
#include <stdlib.h>

#define TRPG_WINDOWS (_WIN32 || _WIN64)

#ifdef TRPG_WINDOWS

	#if defined TRPG_EXPORTS
		#define TRPG_API _declspec(dllexport)
	#else
		#define TRPG_API _declspec(dllimport)
	#endif

#define TASSERT(x) \
{ \
	if ((x) == 0) {abort();} \
}

#endif