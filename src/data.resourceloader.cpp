#include "rl/data.resourceloader.hpp"

#include <Windows.h>





namespace rl
{

	const uint8_t* GetResource(HMODULE hModule, LPCWSTR lpType, LPCWSTR lpName, size_t& size)
	{
		size = 0; // on failure, the size should not have a confusing undefined value

		HRSRC hRsrc = FindResourceW(hModule, lpName, lpType);
		if (hRsrc == NULL)
			return nullptr;

		HGLOBAL hGlobal = LoadResource(hModule, hRsrc);
		if (hGlobal == NULL)
			return nullptr;

		const uint8_t* pData = (const uint8_t*)LockResource(hGlobal);
		if (pData == NULL)
			return nullptr;

		size = SizeofResource(hModule, hRsrc);
		return pData;
	}
	
}