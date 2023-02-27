#pragma once
#ifndef ROBINLE_DLL_PIXEL_WINDOW__PRIVATECORE
#define ROBINLE_DLL_PIXEL_WINDOW__PRIVATECORE



#include <rl/dll/rlPixelWindow/Core.h>
#include <rl/dll/rlPixelWindow/Definitions.h>

#include <Windows.h>



namespace internal
{



	/// <summary>
	/// Set the last error to PXWIN_ERROR_SUCCESS
	/// </summary>
	void ResetError();

	/// <summary>
	/// Set the code of the last error.
	/// </summary>
	void SetError(PixelWindowRes iError);
	
	/// <summary>
	/// Get the handle to this instance of the Pixel Window DLL.
	/// </summary>
	/// <returns></returns>
	HINSTANCE GetDLLHandle();

	/// <summary>
	/// * Resets the current error.<para />
	/// * Checks if the instance pointer is "trustable".<para />
	/// * pointer is not trustable --> set "invalid parameter" error.<para />
	/// </summary>
	bool CheckInstance(PixelWindow p);



}



#endif // ROBINLE_DLL_PIXEL_WINDOW__PRIVATECORE