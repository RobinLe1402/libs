#include <rl/dll/rlPixelWindow/Core.h>
#include <rl/dll/rlPixelWindow/Definitions.h>
#include "PrivateCore.hpp"
#include "PrivateIncludes/Window.hpp"

#include <memory>
#include <stdint.h>
#include <Windows.h>

#pragma comment(lib, "version.lib")
#pragma comment(lib, "OpenGL32.lib")

namespace
{
	PixelWindowRes     iError       = PXWIN_ERROR_SUCCESS;
	PixelWindowOSError iLastOSError = ERROR_SUCCESS;


#define ERROR_HEADER(def) .iError = def, .szDefName = #def
	constexpr PixelWindowErrorMsg oErrorMessages[] =
	{
		{
			ERROR_HEADER(PXWIN_ERROR_SUCCESS),
			.szSummary  = "Success",
			.szDetailed = "No error occured."
		},

		{
			ERROR_HEADER(PXWIN_ERROR_INVALID_PARAM),
			.szSummary  = "Invalid parameter",
			.szDetailed = "One or more parameters passed to a function where invalid."
		},

		{
			ERROR_HEADER(PXWIN_ERROR_NOINIT),
			.szSummary  = "Uninitialized",
			.szDetailed =
				"The PixelWindow object passed to a function was not initialized, "
				"but the called function requires an initialized object."
		},

		{
			ERROR_HEADER(PXWIN_ERROR_OSERROR),
			.szSummary  = "OS error",
			.szDetailed = "An operating system command failed."
		}
	};
#undef ERROR_HEADER

}

PXWIN_API PixelWindowVersion rlPixelWindow_GetVersion()
{
	internal::ResetError();

	TCHAR szDLLFilepath[MAX_PATH + 1];
	if (!GetModuleFileName(internal::GetDLLHandle(), szDLLFilepath, MAX_PATH + 1))
	{
		internal::SetError(PXWIN_ERROR_OSERROR);
		return {};
	}

	const DWORD dwSize =
		GetFileVersionInfoSize(szDLLFilepath, NULL);

	if (dwSize == 0)
	{
		internal::SetError(PXWIN_ERROR_OSERROR);
		return {};
	}
	
	auto up_FileVersionInfo = std::make_unique<uint8_t[]>(dwSize);
	LPVOID pFFI = nullptr;
	UINT iLenFFI = 0;

	if (!GetFileVersionInfo(szDLLFilepath, NULL, dwSize, up_FileVersionInfo.get()) ||
		!VerQueryValue(up_FileVersionInfo.get(), TEXT("\\"), &pFFI, &iLenFFI))
	{
		internal::SetError(PXWIN_ERROR_OSERROR);
		return {};
	}

	auto &ffi = *reinterpret_cast<VS_FIXEDFILEINFO *>(pFFI);

	PixelWindowVersion result = ((uint64_t)ffi.dwFileVersionMS << 32) | ffi.dwFileVersionLS;
	return result;
}

PXWIN_API PixelWindowRes rlPixelWindow_GetError()
{
	return iError;
}

const PixelWindowErrorMsg *rlPixelWindow_GetErrorMsg(
	PixelWindowRes iErrorCode)
{
	for (auto &o : oErrorMessages)
	{
		if (o.iError == iErrorCode)
			return &o;
	}
	return nullptr;
}

PXWIN_API PixelWindowOSError rlPixelWindow_GetOSError()
{
	return iLastOSError;
}

PXWIN_API PixelWindowSizeStruct rlPixelWindow_GetMinSize(
	PixelWindowPixelSize iPixelWidth, PixelWindowPixelSize iPixelHeight,
	PixelWindowBool bResizable, PixelWindowBool bMaximizable)
{
	return internal::Window::MinSize(iPixelWidth, iPixelHeight, bResizable, bMaximizable);
}

PXWIN_API PixelWindowRes rlPixelWindow_DefMsgHandler(
	PixelWindow p, PixelWindowMsg msg, PixelWindowArg arg1, PixelWindowArg arg2)
{
	switch (msg)
	{
	case PXWINMSG_TRYDESTROY:
	case PXWINMSG_TRYRESIZE:
	case PXWINMSG_UPDATE:
		return 1;

	default:
		return 0;
	}
}

PixelWindowPixel rlPixelWindow_ARGB(uint32_t iARGB)
{
	PixelWindowPixel result
	{
		.r     = uint8_t(iARGB >> 16),
		.g     = uint8_t(iARGB >> 8),
		.b     = uint8_t(iARGB),
		.alpha = uint8_t(iARGB >> 24)
	};

	return result;
}

PixelWindowPixel rlPixelWindow_RGB(uint32_t iRGB)
{
	PixelWindowPixel result
	{
		.r     = uint8_t(iRGB >> 16),
		.g     = uint8_t(iRGB >> 8),
		.b     = uint8_t(iRGB),
		.alpha = 0xFF
	};

	return result;
}


namespace internal
{
	void SetError(PixelWindowRes iError)
	{
		::iError = iError;
		if (iError == PXWIN_ERROR_OSERROR)
			iLastOSError = GetLastError();
	}
}