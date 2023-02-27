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

#include "Definitions.h"
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

#define PXWIN_VERSION_GET_MAJOR(version) (uint16_t) ((version >> 48) & 0xFFFF)
#define PXWIN_VERSION_GET_MINOR(version) (uint16_t) ((version >> 32) & 0xFFFF)
#define PXWIN_VERSION_GET_PATCH(version) (uint16_t) ((version >> 16) & 0xFFFF)
#define PXWIN_VERSION_GET_BUILD(version) (uint16_t) ( version        & 0xFFFF)

#define PXWIN_MAKE_VERSION(major, minor, patch, build) \
	(PixelWindowVersion)        \
	((uint16_t)(build)        | \
	((uint32_t)(patch) << 16) | \
	((uint64_t)(minor) << 32) | \
	((uint64_t)(major) << 48))

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
/// Get the minimum bitmap size allowed.
/// </summary>
PXWIN_API PixelWindowSizeStruct PXWIN_CONV rlPixelWindow_GetMinSize(
	PixelWindowPixelSize iPixelWidth, PixelWindowPixelSize iPixelHeight,
	PixelWindowBool bResizable, PixelWindowBool bMaximizable);

/// <summary>
/// Default message handler.<br />
/// Does the bare minimum.
/// </summary>
/// <returns></returns>
PXWIN_API PixelWindowRes PXWIN_CONV rlPixelWindow_DefMsgHandler(
	PixelWindow p, PixelWindowMsg msg, PixelWindowArg arg1, PixelWindowArg arg2);

/// <summary>
/// Create a pixel by an ARGB value.
/// </summary>
PXWIN_API PixelWindowPixel PXWIN_CONV rlPixelWindow_ARGB(uint32_t iARGB);

/// <summary>
/// Create a pixel by an RGB value.
/// </summary>
PXWIN_API PixelWindowPixel PXWIN_CONV rlPixelWindow_RGB(uint32_t iRGB);



PXWIN_API PixelWindow PXWIN_CONV rlPixelWindow_Create(PixelWindowProc pUpdateCallback,
	const PixelWindowCreateParams *pParams);
PXWIN_API void PXWIN_CONV rlPixelWindow_Destroy(PixelWindow p);

PXWIN_API void PXWIN_CONV rlPixelWindow_Run(PixelWindow p);

/// <summary>
/// Get the size (in pixels) of a Pixel Window's bitmaps.
/// </summary>
PXWIN_API PixelWindowSizeStruct PXWIN_CONV rlPixelWindow_GetSize(PixelWindow p);

/// <summary>
/// Get the total count of layered bitmaps.
/// </summary>
PXWIN_API PixelWindowLayerID PXWIN_CONV rlPixelWindow_GetLayerCount(PixelWindow p);

/// <summary>
/// Clear a layer (= set all the pixels to fully transparent).
/// </summary>
PXWIN_API void PXWIN_CONV rlPixelWindow_ClearLayer(PixelWindow p, PixelWindowLayerID iLayerID);

/// <summary>
/// Draw a subimage onto a layer.
/// </summary>
/// <param name="pData">The buffer containing the pixel data of the subimage.</param>
/// <param name="iWidth">Width of the subimage.</param>
/// <param name="iHeight">Height of the subimage.</param>
/// <param name="iLayer">The ID of the layer to draw the subimage on.</param>
/// <param name="iX">X coordinate of where to draw the subimage.</param>
/// <param name="iY">Y coordinate of where to draw the subimage.</param>
/// <param name="iAlphaMode">One of the <c>PXWIN_DRAWALPHA_</c> values.</param>
PXWIN_API void PXWIN_CONV rlPixelWindow_Draw(PixelWindow p,
	const PixelWindowPixel *pData, PixelWindowSize iWidth, PixelWindowSize iHeight,
	uint32_t iLayer, PixelWindowPos iX, PixelWindowPos iY, uint8_t iAlphaMode);

/// <summary>
/// Get the background color of a Pixel Window.
/// </summary>
/// <param name="p"></param>
/// <returns></returns>
PXWIN_API PixelWindowPixel PXWIN_CONV rlPixelWindow_GetBackgroundColor(PixelWindow p);

/// <summary>
/// Set the background color of a Pixel Window.
/// </summary>
PXWIN_API void PXWIN_CONV rlPixelWindow_SetBackgroundColor(PixelWindow p, PixelWindowPixel px);


PXWIN_API const wchar_t *PXWIN_CONV rlPixelWindow_GetTitle(PixelWindow p);

/// <summary>
/// Set the title of a Pixel Window
/// </summary>
PXWIN_API void PXWIN_CONV rlPixelWindow_SetTitle(PixelWindow p, const wchar_t *szTitle);



#endif // ROBINLE_PIXEL_WINDOW