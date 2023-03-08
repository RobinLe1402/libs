#pragma once
#ifndef ROBINLE_DLL_PIXEL_WINDOW__PRIVATECORE
#define ROBINLE_DLL_PIXEL_WINDOW__PRIVATECORE



#include <rl/dll/rlPixelWindow/Core.h>
#include <rl/dll/rlPixelWindow/Definitions.h>
#include <rl/dll/rlPixelWindow/Types.h>

#include <type_traits>
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



	/// <summary>
	/// Create a <c>PixelWindowArg</c> out of a single generic value.<para/>
	/// Only available if type is not too large and convertable to unsigned int.
	/// </summary>
	template <typename T>
	inline typename std::enable_if<
		// check for compatible size
		sizeof(T) <= sizeof(PixelWindowArg) &&
		// check if convertable to transport type (unsigned integer)
		(std::is_fundamental<T>::value || std::is_pointer<T>::value),
		PixelWindowArg>::type MakeArg(T val)
	{
		return reinterpret_cast<PixelWindowArg>(val);
	}
	
	/// <summary>Create a <c>PixelWindowArg</c> out of two generic values.</summary>
	inline PixelWindowArg MakeArgFrom2(uint32_t iLow, uint32_t iHigh = 0)
	{
		return
			(PixelWindowArg(iLow)) |
			(PixelWindowArg(iHigh) << 32);
	}

	/// <summary>
	/// Create a <c>PixelWindowArg</c> out of four generic values.<para/>
	/// <c>i1</c> is the least significant, <c>i4</c> is the most significant.
	/// </summary>
	inline PixelWindowArg MakeArgFrom4(
		uint16_t i1, uint16_t i2 = 0, uint16_t i3 = 0, uint16_t i4 = 0)
	{
		return
			(PixelWindowArg(i1))       |
			(PixelWindowArg(i2) << 16) |
			(PixelWindowArg(i3) << 32) |
			(PixelWindowArg(i4) << 48);
	}

	/// <summary>
	/// Create a <c>PixelWindowArg</c> out of eight generic values.<para/>
	/// <c>i1</c> is the least significant, <c>i8</c> is the most significant.
	/// </summary>
	inline PixelWindowArg MakeArgFrom8(
		uint8_t i1,     uint8_t i2 = 0, uint8_t i3 = 0, uint8_t i4 = 0,
		uint8_t i5 = 0, uint8_t i6 = 0, uint8_t i7 = 0, uint8_t i8 = 0)
	{
		return
			(PixelWindowArg(i1))       |
			(PixelWindowArg(i2) << 8)  |
			(PixelWindowArg(i3) << 16) |
			(PixelWindowArg(i4) << 24) |
			(PixelWindowArg(i5) << 32) |
			(PixelWindowArg(i6) << 40) |
			(PixelWindowArg(i7) << 48) |
			(PixelWindowArg(i8) << 56);
	}



	inline bool operator==(
		const PixelWindowSizeStruct &lhs,
		const PixelWindowSizeStruct &rhs
		)
	{
		return (memcmp(&lhs, &rhs, sizeof(PixelWindowSizeStruct)) == 0);
	}

	inline bool operator==(
		const PixelWindowPixelSizeStruct &lhs,
		const PixelWindowPixelSizeStruct &rhs
		)
	{
		return (memcmp(&lhs, &rhs, sizeof(PixelWindowPixelSizeStruct)) == 0);
	}



}



#endif // ROBINLE_DLL_PIXEL_WINDOW__PRIVATECORE