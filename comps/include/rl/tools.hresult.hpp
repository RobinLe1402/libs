/***************************************************************************************************
 FILE:	tools.hresult.hpp
 CPP:	<n/a>
 DESCR:	Some HRESULT related functions
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_TOOLS_HRESULT
#define ROBINLE_TOOLS_HRESULT





#include <string>
#include <Windows.h>



//==================================================================================================
// DECLARATION
namespace rl
{
	
	void GenerateHResultString(std::string& dest, HRESULT hr,
		const char* szPrefix = nullptr, const char* szSuffix = nullptr)
	{
		char szErrorText[256] = {};
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szErrorText, 0256, NULL);

		if (strlen(szErrorText) == 0)
			strcat_s(szErrorText, "Unknown error");

		char szErrorCode[] = "[0x00000000] ";
		static char cHEX[] = "0123456789ABCDEF";
		for (uint8_t i = 0; i < sizeof(hr) * 2; i++)
		{
			szErrorCode[i + 3] = cHEX[(hr >> ((7 - i) * 4)) & 0x0F];
		}

		size_t len = strlen(szErrorText) + strlen(szErrorCode);
		if (szPrefix != nullptr)
			len += strlen(szPrefix);
		if (szSuffix != nullptr)
			len += strlen(szSuffix);

		dest.clear();
		dest.reserve(len);
		if (szPrefix != nullptr)
			dest += szPrefix;
		dest += szErrorCode;
		dest += szErrorText;
		if (szSuffix != nullptr)
			dest += szSuffix;
	}

	void ThrowHResultException(HRESULT hr,
		const char* szPrefix = nullptr, const char* szSuffix = nullptr)
	{
		std::string sError;
		GenerateHResultString(sError, hr, szPrefix, szSuffix);
		throw std::exception(sError.c_str());
	}
	
}





#endif // ROBINLE_TOOLS_HRESULT