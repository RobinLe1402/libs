/***************************************************************************************************
 FILE:	rlPixelWindow++/Core.hpp
 DLL:	rlPixelWindow.dll
 DESCR:	C++ wrapper around the core functionality of the PixelWindow DLL.
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DLL_PIXELWINDOW_CPP
#define ROBINLE_DLL_PIXELWINDOW_CPP





#include <rl/dll/rlPixelWindow/Core.h>

namespace rl
{
	namespace PixelWindowDLL
	{



		inline auto GetVersion() { return rlPixelWindow_GetVersion(); }
		inline auto GetError() { return rlPixelWindow_GetError(); }
		inline void SetError(PixelWindowRes iError) { rlPixelWindow_SetError(iError); }



	}
}





#endif // ROBINLE_DLL_PIXELWINDOW_CPP