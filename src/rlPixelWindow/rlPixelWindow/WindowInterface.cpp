#include <rl/dll/rlPixelWindow/Core.h>
#include <rl/dll/rlPixelWindow/Types.h>
#include "PrivateIncludes/Window.hpp"
#include "PrivateCore.hpp"

namespace
{
	internal::Window *IntfToImpl(PixelWindow p)
	{
		return reinterpret_cast<internal::Window *>(p);
	}
}

PXWIN_API PixelWindow rlPixelWindow_Create(PixelWindowProc pUpdateCallback,
	const PixelWindowCreateParams *pParams)
{
	auto &oInst = internal::Window::instance();

	internal::ResetError();

	oInst.create(pUpdateCallback, pParams);

	if (oInst)
		return oInst.intfPtr();
	else
		return nullptr;
}

PXWIN_API void rlPixelWindow_Destroy(PixelWindow p)
{
	internal::ResetError();

	if (!p)
		return;

	IntfToImpl(p)->destroy();
}

void rlPixelWindow_Run(PixelWindow p)
{
	internal::ResetError();

	if (!p)
	{
		internal::SetError(PXWIN_ERROR_INVALID_PARAM);
		return;
	}


	IntfToImpl(p)->run();
}

void rlPixelWindow_Draw(PixelWindow p,
	const PixelWindowPixel *pData, PixelWindowSize iWidth, PixelWindowSize iHeight, 
	uint32_t iLayer, PixelWindowPos iX, PixelWindowPos iY, uint8_t iFlags, uint8_t iAlphaMode)
{
	internal::ResetError();

	if (!p)
	{
		internal::SetError(PXWIN_ERROR_INVALID_PARAM);
		return;
	}

	IntfToImpl(p)->draw(pData, iWidth, iHeight, iLayer, iX, iY, iFlags, iAlphaMode);
}

void rlPixelWindow_SetBackgroundColor(PixelWindow p, PixelWindowPixel px)
{
	internal::ResetError();

	if (!p)
	{
		internal::SetError(PXWIN_ERROR_INVALID_PARAM);
		return;
	}

	IntfToImpl(p)->setBackgroundColor(px);
}
