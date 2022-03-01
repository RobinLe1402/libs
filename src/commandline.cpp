#include "rl/commandline.hpp"

#include <Shlwapi.h>
#include <string.h>
#include <Windows.h>





namespace rl
{
	
	bool CmdGetPathLen(const wchar_t* szCMD, int* pLen)
	{
		if (szCMD[0] == L'\0' || szCMD[0] == L' ')
			return false;

		*pLen = 0;
		const size_t len = wcslen(szCMD);

		if (szCMD[0] != L'"')
		{
			while (*pLen < len && szCMD[*pLen] != L' ')
				(*pLen)++;
			return true;
		}
		else
		{
			if (szCMD[1] == L'\0' || szCMD[1] == L'"')
				return false;

			*pLen = 1;
			while (*pLen < len && szCMD[*pLen] != L'"')
				(*pLen)++;

			if (szCMD[*pLen] != L'"')
			{
				*pLen = 0;
				return false;
			}

			(*pLen)++; // include closing quotes in path length
			return true;
		}
	}

	bool CmdGetInOutPaths(const wchar_t* szCMD, wchar_t(&szPathIn)[MAX_PATH + 1],
		wchar_t(&szPathOut)[MAX_PATH + 1])
	{
		const size_t len = wcslen(szCMD);
		int iParamOffset = 0;
		rl::CmdGetPathLen(szCMD, &iParamOffset);
		iParamOffset++; // skip space

		int iInPathLen = 0;
		if (!rl::CmdGetPathLen(szCMD + iParamOffset, &iInPathLen))
			return false; // no input path
		if ((size_t)iParamOffset + iInPathLen == len)
			return false; // no more space for output path

		int iOutPathLen = 0;
		if (!rl::CmdGetPathLen(szCMD + iParamOffset + iInPathLen + 1, &iOutPathLen))
			return false; // no output path
		if ((size_t)iParamOffset + iInPathLen + 1 + iOutPathLen < len)
			return false; // additional parameters

		memcpy_s(szPathIn, sizeof(wchar_t) * (MAX_PATH + 1), szCMD + iParamOffset,
			sizeof(wchar_t) * iInPathLen);
		szPathIn[iInPathLen] = L'\0'; // append terminating zero
		PathUnquoteSpacesW(szPathIn); // remove quotes

		memcpy_s(szPathOut, sizeof(wchar_t) * (MAX_PATH + 1),
			szCMD + iParamOffset + iInPathLen + 1, sizeof(wchar_t) * iOutPathLen);
		szPathOut[iOutPathLen] = L'\0'; // append terminating zero
		PathUnquoteSpacesW(szPathOut); // remove quotes

		return true;
	}
	
}