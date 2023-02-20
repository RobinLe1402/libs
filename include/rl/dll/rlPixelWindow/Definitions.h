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
#define PXWIN_ERROR_NOINIT        2 // Window wasn't initialized before the current call.
#define PXWIN_ERROR_OSERROR       3 // An OS command failed.





#define PXWINMSG_UPDATE       0  // A regular update call. Update inner state and draw changes.

#define PXWINMSG_CREATE       1  // Right after the window was created (but still hidden).

#define PXWINMSG_SHOW         2  // Right before window is shown.

#define PXWINMSG_TRYDESTROY   3  // Test if window can be closed.
#define PXWINMSG_DESTROY      4  // Right after window is closed.

#define PXWINMSG_TRYRESIZE    5  // Test if window can be resized.
#define PXWINMSG_RESIZE       6  // Right after the window size changed.

#define PXWINMSG_GAINEDFOCUS  7  // The window just received keyboard focus.
#define PXWINMSG_LOSTFOCUS    8  // The window just lost keyboard focus.

#define PXWINMSG_MOUSELEAVE   9  // The mouse cursor just left the client area.

#define PXWINMSG_MINIMIZED    10 // The window was minimized.
#define PXWINMSG_MAXIMIZED    11 // The window was maximized.
#define PXWINMSG_RESTORED     12 // The window was restored.

#define PXWINMSG_FILEDRAGOVER 13 // A file/multiple files are dragged over the window.
#define PXWINMSG_FILEDROP     14 // A file/multiple files have been dropped onto the window.

// Ideas:
// * HIDE // The window was hidden.
// * MOVE // The window was moved.





#define PXWIN_CREATE_MAXIMIZABLE 1 // Is the window maximizable?
#define PXWIN_CREATE_RESIZABLE   2 // Is the window resizable?

#define PXWIN_UPDATEREASON_REGULAR 0 // Regular update call
#define PXWIN_UPDATEREASON_RESIZE  1 // Window was resized
#define PXWIN_UPDATEREASON_START   2 // First time drawing before window is shown





#endif // ROBINLE_DLL_PIXEL_WINDOW__DEFINITIONS