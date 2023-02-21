#include "PrivateCore.hpp"

#include <Windows.h>

namespace
{
	HINSTANCE hInst = NULL;
}

namespace internal
{
	void ResetError()
	{
		internal::SetError(PXWIN_ERROR_SUCCESS);
	}

	HINSTANCE GetDLLHandle()
	{
		return hInst;
	}

}

extern "C" __declspec(dllexport) BOOL APIENTRY DllMain(
	HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}
