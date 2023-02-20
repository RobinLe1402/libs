/***************************************************************************************************
 FILE:	rlPixelWindow/Core.h
 DLL:	rlPixelWindow.dll
 DESCR:	The core functionality of the PixelWindow DLL.
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DLL_PIXEL_WINDOW
#define ROBINLE_DLL_PIXEL_WINDOW

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#ifdef DLLEXPORT_PIXELWINDOW
#define PXWIN_API EXTERN_C __declspec(dllexport)
#else
#define PXWIN_API EXTERN_C __declspec(dllimport)
#endif

#define PXWIN_CONV __stdcall





//==================================================================================================
// INCLUDES

#include "Types.h"

#include <stdint.h>





//==================================================================================================
// DECLARATION

/// <summary>
/// Get the version number of the rlPixelWindow DLL.
/// </summary>
/// <returns>
/// The version number.<para/>
/// Starting from the most significant side, 16 bits make up one part of the version number:<para/>
/// <c>0xFFFF000000000000</c> = Major version<para/>
/// <c>0x0000FFFF00000000</c> = Minor version<para/>
/// <c>0x00000000FFFF0000</c> = Patch level<para/>
/// <c>0x000000000000FFFF</c> = Build number<para/>
/// <para/>
/// Example: v1.8.4.2 = <c>0x0001'0008'0004'0002</c>
/// </returns>
PXWIN_API PixelWindowVersion PXWIN_CONV rlPixelWindow_GetVersion();

#define PXWIN_VERSION_GET_MAJOR(version) (uint8_t)  (version >> 48)
#define PXWIN_VERSION_GET_MINOR(version) (uint8_t) ((version >> 32) & 0xFF)
#define PXWIN_VERSION_GET_PATCH(version) (uint8_t) ((version >> 16) & 0xFF)
#define PXWIN_VERSION_GET_BUILD(version) (uint8_t) ( version        & 0xFF)

/// <summary>
/// Get the code of the last error.
/// </summary>
/// <returns>One of the error codes defined as <c>PXWIN_ERROR_</c>.</returns>
PXWIN_API PixelWindowRes PXWIN_CONV rlPixelWindow_GetError();

/// <summary>
/// Get error messages associate with an error code.
/// </summary>
PXWIN_API const PixelWindowErrorMsg *PXWIN_CONV rlPixelWindow_GetErrorMsg(
	PixelWindowRes iErrorCode);

/// <summary>
/// Get the code of the last OS error.
/// </summary>
/// <returns>[Windows] One of the error codes defined as <c>ERROR_</c>.</returns>
PXWIN_API PixelWindowOSError PXWIN_CONV rlPixelWindow_GetOSError();

/// <summary>
/// Default message handler.<br />
/// Does the bare minimum.
/// </summary>
/// <returns></returns>
PXWIN_API PixelWindowRes PXWIN_CONV rlPixelWindow_DefMsgHandler(
	PixelWindow p, PixelWindowMsg msg, PixelWindowArg arg1, PixelWindowArg arg2);



PXWIN_API PixelWindow PXWIN_CONV rlPixelWindow_Create(PixelWindowProc pUpdateCallback,
	const PixelWindowCreateParams *pParams);
PXWIN_API void PXWIN_CONV rlPixelWindow_Destroy(PixelWindow p);

PXWIN_API void PXWIN_CONV rlPixelWindow_Run(PixelWindow p);



#endif // ROBINLE_PIXEL_WINDOW