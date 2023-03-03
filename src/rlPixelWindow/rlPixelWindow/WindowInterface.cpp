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

PixelWindow rlPixelWindow_Create(PixelWindowProc pUpdateCallback,
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

void rlPixelWindow_Destroy(PixelWindow p)
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

PixelWindowSizeStruct rlPixelWindow_GetSize(PixelWindow p)
{
	if (!internal::CheckInstance(p))
		return {};

	return IntfToImpl(p)->getSize();
}

PixelWindowPixelSizeStruct rlPixelWindow_GetPixelSize(PixelWindow p)
{
	if (!internal::CheckInstance(p))
		return {};

	return IntfToImpl(p)->getPixelSize();
}

PixelWindowBool rlPixelWindow_SetPixelSize(PixelWindow p, PixelWindowPixelSizeStruct oPixelSize)
{
	if (!internal::CheckInstance(p))
		return false;

	return IntfToImpl(p)->setPixelSize(oPixelSize);
}

PixelWindowLayerID rlPixelWindow_GetLayerCount(PixelWindow p)
{
	if (!internal::CheckInstance(p))
		return 0;

	return IntfToImpl(p)->getLayerCount();
}

void rlPixelWindow_ClearLayer(PixelWindow p, PixelWindowLayerID iLayerID)
{
	if (!internal::CheckInstance(p))
		return;

	IntfToImpl(p)->clearLayer(iLayerID);
}

void rlPixelWindow_Draw(PixelWindow p,
	const PixelWindowPixel *pData, PixelWindowSize iWidth, PixelWindowSize iHeight, 
	uint32_t iLayer, PixelWindowPos iX, PixelWindowPos iY, uint8_t iAlphaMode)
{
	if (!internal::CheckInstance(p))
		return;

	IntfToImpl(p)->draw(pData, iWidth, iHeight, iLayer, iX, iY, iAlphaMode);
}

PixelWindowPixel rlPixelWindow_GetBackgroundColor(PixelWindow p)
{
	if (!internal::CheckInstance(p))
		return {};

	return IntfToImpl(p)->getBackgroundColor();
}

void rlPixelWindow_SetBackgroundColor(PixelWindow p, PixelWindowPixel px)
{
	if (!internal::CheckInstance(p))
		return;

	IntfToImpl(p)->setBackgroundColor(px);
}

const wchar_t *rlPixelWindow_GetTitle(PixelWindow p)
{
	if (!internal::CheckInstance(p))
		return nullptr;

	return IntfToImpl(p)->getTitle();
}

void rlPixelWindow_SetTitle(PixelWindow p, const wchar_t *szTitle)
{
	if (!internal::CheckInstance(p))
		return;

	IntfToImpl(p)->setTitle(szTitle);
}
