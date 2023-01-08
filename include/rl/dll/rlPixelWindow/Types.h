#pragma once
#ifndef ROBINLE_DLL_PIXEL_WINDOW__TYPES
#define ROBINLE_DLL_PIXEL_WINDOW__TYPES





#include <stdint.h>





// opaque pointer
typedef struct tagPixelWindowOpaquePointer
{
	int iUnused;
} *PixelWindow;

typedef uint32_t PixelWindowMsg; // Callback message.
typedef uint64_t PixelWindowArg; // Callback argument.
typedef uint64_t PixelWindowRes; // Callback result/error code.

typedef PixelWindowRes(*PixelWindowProc)(PixelWindow p, PixelWindowMsg msg,
	PixelWindowArg arg1, PixelWindowArg arg2);


typedef uint32_t PixelWindowSize;
typedef uint16_t PixelWindowPixelSize;
typedef uint32_t PixelWindowPos;

typedef struct tagPixelWindowCreateParams
{
	PixelWindowSize      iWidth;       // The width of the canvas, in pixels.
	PixelWindowSize      iHeight;      // The height of the canvas, in pixels.

	PixelWindowPixelSize iPixelWidth;  // The width of a single virtual pixel, in actual pixels.
	PixelWindowPixelSize iPixelHeight; // The height of a single virtual pixel, in actual pixels.

	uint32_t             iFlags;       // Combination of PXWIN_CREATE_[...] flags.
	intptr_t             iUserData;    // Custom user data.
} PixelWindowCreateParams;





#endif // ROBINLE_DLL_PIXEL_WINDOW__TYPES