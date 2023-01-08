#include <rl/dll/rlPixelWindow/Core.h>
#include <rl/dll/rlPixelWindow/Types.h>
#include "PrivateIncludes/Window.hpp"

PXWIN_API PixelWindow rlPixelWindow_Create(PixelWindowProc pUpdateCallback,
	const PixelWindowCreateParams* pParams)
{
	auto &oInst = internal::Window::instance();

	oInst.create(pUpdateCallback, pParams);

	if (oInst)
		return reinterpret_cast<PixelWindow>(&oInst);
	else
		return nullptr;
}

PXWIN_API void rlPixelWindow_Destroy(PixelWindow p)
{
	if (!p)
		return;

	reinterpret_cast<internal::Window*>(p)->destroy();
}
