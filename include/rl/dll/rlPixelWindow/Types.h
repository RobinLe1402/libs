#pragma once
#ifndef ROBINLE_DLL_PIXEL_WINDOW__TYPES
#define ROBINLE_DLL_PIXEL_WINDOW__TYPES





#include <stdint.h>





typedef uint64_t PixelWindowVersion;

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

const PixelWindowPixel px = PXWIN_COLOR_BLANK;

typedef uint16_t PixelWindowSize;
typedef uint16_t PixelWindowPixelSize;
typedef uint32_t PixelWindowPos;

typedef struct tagPixelWindowCreateParams
{
	PixelWindowSize      iWidth;       // The width of the canvas, in pixels.
	PixelWindowSize      iHeight;      // The height of the canvas, in pixels.

	PixelWindowPixelSize iPixelWidth;  // The width of a single virtual pixel, in actual pixels.
	PixelWindowPixelSize iPixelHeight; // The height of a single virtual pixel, in actual pixels.

	uint32_t             iExtraLayers; // The count of layers used in addition to the base layer
	PixelWindowPixel     pxBackground; // The background color.

	uint32_t             iFlags;       // Combination of PXWIN_CREATE_[...] flags.
	intptr_t             iUserData;    // Custom user data.
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