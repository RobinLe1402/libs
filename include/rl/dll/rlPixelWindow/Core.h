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
/// Starting from the most significant byte, each byte is one part of the version number:<para/>
/// <c>0xFF000000</c> = Major version<para/>
/// <c>0x00FF0000</c> = Minor version<para/>
/// <c>0x0000FF00</c> = Patch level<para/>
/// <c>0x000000FF</c> = Build number<para/>
/// <para/>
/// Example: v1.8.4.2 = <c>0x01080402</c>
/// </returns>
PXWIN_API uint32_t rlPixelWindow_GetVersion();

/// <summary>
/// Get the code of the last error.
/// </summary>
/// <returns>One of the error codes defined as <c>PXWIN_ERROR_</c>.</returns>
PXWIN_API PixelWindowRes rlPixelWindow_GetError();

/// <summary>
/// Set the code of the last error.
/// </summary>
PXWIN_API void rlPixelWindow_SetError(PixelWindowRes iError);
// TODO: make private?



PXWIN_API PixelWindow rlPixelWindow_Create(PixelWindowProc pUpdateCallback,
	const PixelWindowCreateParams* pParams);
PXWIN_API void rlPixelWindow_Destroy(PixelWindow p);

PXWIN_API void rlPixelWindow_Show(PixelWindow p);



#endif // ROBINLE_PIXEL_WINDOW