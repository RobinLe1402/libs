#pragma once
#ifndef ROBINLE_CONSOLE_BASE
#define ROBINLE_CONSOLE_BASE





#ifdef __cplusplus
	#define RL_CDECL "C"
#else
	#define RL_CDECL
	#include <stdbool.h>
#endif // __cplusplus

#ifdef RLCONSOLE_DLL_EXPORT
	#define RLCONSOLE_API extern RL_CDECL __declspec(dllexport)
#else
	#define RLCONSOLE_API extern RL_CDECL __declspec(dllimport)
	#pragma comment(lib, "rlConsole.lib")
#endif // RLCONSOLE_DLL_EXPORT



#include <stdint.h>



typedef	uint32_t	rlCon_char;





#endif // ROBINLE_CONSOLE_BASE