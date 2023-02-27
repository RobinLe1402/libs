#pragma once
#ifndef ROBINLE_DLL_PIXEL_WINDOW__TYPES
#define ROBINLE_DLL_PIXEL_WINDOW__TYPES





#include "Definitions.h"

#include <stdint.h>
#include <Windows.h>





typedef uint64_t PixelWindowVersion;

// opaque pointer
typedef struct tagPixelWindowOpaquePointer
{
	int iUnused;
} *PixelWindow;

typedef uint32_t PixelWindowMsg; // Callback message.
typedef uint64_t PixelWindowArg; // Callback argument.
typedef uint64_t PixelWindowRes; // Callback result/error code.

typedef PixelWindowRes(PXWIN_CONV *PixelWindowProc)(PixelWindow p, PixelWindowMsg msg,
	PixelWindowArg arg1, PixelWindowArg arg2);
typedef void(PXWIN_CONV *PixelWindowOSProc)(PixelWindow p,
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


/// <summary>
/// Pixel data. Compatible with OpenGL (RGBA).
/// </summary>
typedef struct tagPixelWindowPixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t alpha;
} PixelWindowPixel;

//                                           RED   GREEN BLUE  ALPHA
#define PXWIN_COLOR_BLANK (PixelWindowPixel{ 0x00, 0x00, 0x00, 0x00 })
#define PXWIN_COLOR_WHITE (PixelWindowPixel{ 0xFF, 0xFF, 0xFF, 0xFF })
#define PXWIN_COLOR_BLACK (PixelWindowPixel{ 0x00, 0x00, 0x00, 0xFF })
#define PXWIN_COLOR_RED   (PixelWindowPixel{ 0xFF, 0x00, 0x00, 0xFF })
#define PXWIN_COLOR_GREEN (PixelWindowPixel{ 0x00, 0xFF, 0x00, 0xFF })
#define PXWIN_COLOR_BLUE  (PixelWindowPixel{ 0x00, 0x00, 0xFF, 0xFF })

typedef uint8_t  PixelWindowBool;
typedef uint16_t PixelWindowSize;
typedef uint16_t PixelWindowPixelSize;
typedef uint32_t PixelWindowPos;
typedef uint16_t PixelWindowLayerID;

typedef struct tagPixelWindowSizeStruct
{
	PixelWindowSize iWidth;
	PixelWindowSize iHeight;
} PixelWindowSizeStruct;


typedef struct tagPixelWindowCreateParams
{
	PixelWindowSizeStruct oCanvasSize;  // The size of the canvas, in pixels
	PixelWindowSizeStruct oMinSize;     // The maximum size of the canvas, in pixels
	PixelWindowSizeStruct oMaxSize;     // The maximum size of the canvas, in pixels

	PixelWindowPixelSize  iPixelWidth;  // The width of a single virtual pixel, in actual pixels.
	PixelWindowPixelSize  iPixelHeight; // The height of a single virtual pixel, in actual pixels.

	PixelWindowLayerID    iExtraLayers; // The count of layers used in addition to the base layer
	PixelWindowPixel      pxBackground; // The background color.

	uint32_t              iFlags;       // Combination of PXWIN_CREATE_[...] flags.
	intptr_t              iUserData;    // Custom user data.

	const wchar_t        *szTitle;      // The initial title of the window.

	PixelWindowOSProc     fnOSCallback; // Callback for OS messages.
} PixelWindowCreateParams;



typedef struct tagPixelWindowUpdateParams
{
	float   fElapsedTime;
	uint8_t iUpdateReason;
} PixelWindowUpdateParams;



typedef struct tagPixelWindowErrorMsg
{
	PixelWindowRes iError;  // the error value
	const char *szDefName;  // the defined name for this error
	const char *szSummary;  // a quick summary of the error
	const char *szDetailed; // a more detailed description of the error
} PixelWindowErrorMsg;

typedef uint32_t PixelWindowOSError;





#endif // ROBINLE_DLL_PIXEL_WINDOW__TYPES