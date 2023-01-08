#pragma once
#ifndef ROBINLE_DLL_PIXEL_WINDOW__DEFINITIONS
#define ROBINLE_DLL_PIXEL_WINDOW__DEFINITIONS





#if defined(_WIN64) // 64-Bit Windows
	#define PXWIN_64BIT
#elif defined(_WIN32) // 32-Bit Windows
	#define PXWIN_32BIT
#else // neither 32-bit nor 64-bit Windows
	#error rlPixelWindow is only supported for Windows 32/64 bit.
#endif // OS check





#define PXWIN_ERROR_SUCCESS       0 // No error.
#define PXWIN_ERROR_INVALID_PARAM 1 // Invalid function parameter.





#define PXWINMSG_UPDATE       0  // A regular update call.

#define PXWINMSG_TRYCREATE    1  // Test if window can be created.
#define PXWINMSG_CREATE       2  // Right after the window was created (but still hidden).

#define PXWINMSG_SHOW         3  // Right before window is shown.

#define PXWINMSG_TRYDESTROY   4  // Test if window can be closed.
#define PXWINMSG_DESTROY      5  // Right after window is closed.

#define PXWINMSG_TRYRESIZE    6  // Test if window can be resized.
#define PXWINMSG_RESIZE       7  // Right after the window size changed.

#define PXWINMSG_GOTFOCUS     8  // The window just received keyboard focus.
#define PXWINMSG_LOSTFOCUS    9  // The window just lost keyboard focus.

#define PXWINMSG_MOUSELEAVE   10 // The mouse cursor just left the client area.

#define PXWINMSG_MINIMIZED    11 // The window was minimized.
#define PXWINMSG_MAXIMIZED    12 // The window was maximized.
#define PXWINMSG_RESTORED     13 // The window was restored.

#define PXWINMSG_FILEDRAGOVER 14 // A file/multiple files are dragged over the window.
#define PXWINMSG_FILEDROP     15 // A file/multiple files have been dropped onto the window.

// Ideas:
// * HIDE // The window was hidden.
// * MOVE // The window was moved.





#define PXWIN_CREATE_MAXIMIZABLE 1 // Is the window maximizable?
#define PXWIN_CREATE_RESIZABLE   2 // Is the window resizable?





#endif // ROBINLE_DLL_PIXEL_WINDOW__DEFINITIONS