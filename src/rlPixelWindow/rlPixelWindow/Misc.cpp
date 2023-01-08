#include <rl/dll/rlPixelWindow/Core.h>
#include <rl/dll/rlPixelWindow/Definitions.h>

#include <stdint.h>

namespace
{
	constexpr uint8_t iVersionMajor = 1;
	constexpr uint8_t iVersionMinor = 0;
	constexpr uint8_t iPatchLevel   = 0;
	constexpr uint8_t iBuildNumber  = 0;

	constexpr uint32_t iVersion =
		((uint32_t)iVersionMajor << 24) |
		((uint32_t)iVersionMinor << 16) |
		((uint32_t)iPatchLevel   << 8)  |
		((uint32_t)iBuildNumber);


	PixelWindowRes iError = PXWIN_ERROR_SUCCESS;
}

PXWIN_API uint32_t rlPixelWindow_GetVersion()
{
	return iVersion;
}

PXWIN_API PixelWindowRes rlPixelWindow_GetError()
{
	return iError;
}

PXWIN_API void rlPixelWindow_SetError(PixelWindowRes iError)
{
	::iError = iError;
}
