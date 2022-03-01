#include "converter.hpp"

#include <stdint.h>
#include <Windows.h>





namespace rl
{


	struct LANGANDCODEPAGE
	{
		WORD wLanguage;
		WORD wCodePage;
	};

	union LANGANDCODEPAGEU
	{
		LANGANDCODEPAGE lcp;
		DWORD val;
	};


	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	bool GetVersionStr(uint8_t* pVerData, char(&szStrPath)[75], const char* szStrName,
		WORD wCP, std::wstring& sDest)
	{
		strcat_s(szStrPath, szStrName);

		char* sz = nullptr;
		UINT iSize = 0;

		bool result = VerQueryValueA(pVerData, szStrPath, (LPVOID*)&sz, &iSize);

		if (result)
		{
			const auto iLen = MultiByteToWideChar(wCP, NULL, sz, -1, NULL, 0);
			sDest.resize((size_t)iLen - 1);
			MultiByteToWideChar(wCP, NULL, sz, -1, &sDest[0], iLen);
		}

		szStrPath[25] = 0; // "reset" path string
		return result;
	}

	void GenVerStrPath(LANGANDCODEPAGE lang, const char* szStrName, char(&szResult)[75])
	{
		strcpy_s(szResult, R"(\StringFileInfo\)");

		sprintf_s(szResult + strlen(szResult), 75 - strlen(szResult), "%04x%04x",
			lang.wLanguage, lang.wCodePage);
		strcat_s(szResult, R"(\)");
		strcat_s(szResult, szStrName);
	}

	// read or guess the language of a VERSIONINFO resource
	LANGANDCODEPAGE GetVersionLang(uint8_t* pVerData)
	{
		LANGANDCODEPAGEU langcp = {};
		UINT iSize = 0;
		char szPath[75] = {};
		std::wstring sStub;
		LPVOID p = nullptr;


		// default - 1252
		langcp.lcp.wLanguage = 0x0409;
		langcp.lcp.wCodePage = 0x04E4;
		GenVerStrPath(langcp.lcp, "FileDescription", szPath);

		if (VerQueryValueA(pVerData, szPath, &p, &iSize) && iSize > 0)
			return langcp.lcp;

		// not 1252 --> read from translation table
		LANGANDCODEPAGE* pLangcp = nullptr;
		BOOL b = VerQueryValueA(pVerData, R"(\VarFileInfo\Translation)",
			(LPVOID*)&pLangcp, &iSize);

		if (b && iSize > 0) // successfull --> result found
			return *pLangcp;

		// no translation table/invalid data
		langcp = {};
		return langcp.lcp;
	}

	bool MicrosoftFON::GetVersionInfo(const wchar_t* szFON, VERSION_INFO& ver)
	{
		bool result = false;


		DWORD dwHandle = 0;
		const DWORD dwSize = GetFileVersionInfoSizeW(szFON, &dwHandle);
		uint8_t* pBuffer = NULL;
		UINT iSize = 0;
		if (dwSize != NULL)
		{
			uint8_t* pVerData = new uint8_t[dwSize];
			if (dwHandle == 0 && ::GetFileVersionInfoW(szFON, dwHandle, dwSize, pVerData))
			{
				// see https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-verqueryvaluea
				if (VerQueryValueA(pVerData, R"(\)", (LPVOID*)&pBuffer, &iSize))
				{
					if (iSize)
					{
						VS_FIXEDFILEINFO* pVerInfo = (VS_FIXEDFILEINFO*)pBuffer;

#define MAKEUINT64(ls, ms) uint64_t(ls | ((uint64_t)ms << 32))
						if (pVerInfo->dwSignature == 0xfeef04bd)
						{
							ver.iFileVersion = MAKEUINT64(pVerInfo->dwFileVersionLS,
								pVerInfo->dwFileVersionMS);
							ver.iProductVersion = MAKEUINT64(pVerInfo->dwProductVersionLS,
								pVerInfo->dwProductVersionMS);
							ver.iFileFlagsMask = pVerInfo->dwFileFlagsMask;
							ver.iFileFlags = pVerInfo->dwFileFlags;
							ver.iFileOS = pVerInfo->dwFileOS;
							ver.iFileType = pVerInfo->dwFileType;
							ver.iFileSubtype = pVerInfo->dwFileSubtype;

							result = true;
						}
#undef MAKEUINT64
					}
				}
				if (result)
				{
					LANGANDCODEPAGE langcp = GetVersionLang(pVerData);
					result = (langcp.wCodePage != 0 || langcp.wLanguage != 0);

					if (result)
					{
						char szPath[75] = { 0 };
						GenVerStrPath(langcp, "", szPath);
						const size_t iCutoff = strlen(szPath);

						// FileDescription
						GetVersionStr(pVerData, szPath, "FileDescription", langcp.wCodePage,
							ver.sFileDescription);

						// Comments
						GetVersionStr(pVerData, szPath, "Comments", langcp.wCodePage,
							ver.sComments);

						// CompanyName
						GetVersionStr(pVerData, szPath, "CompanyName", langcp.wCodePage,
							ver.sCompanyName);

						// FileVersion
						GetVersionStr(pVerData, szPath, "FileVersion", langcp.wCodePage,
							ver.sFileVersion);

						// InternalName
						GetVersionStr(pVerData, szPath, "InternalName", langcp.wCodePage,
							ver.sInternalName);

						// LegalCopyright
						GetVersionStr(pVerData, szPath, "LegalCopyright", langcp.wCodePage,
							ver.sLegalCopyright);

						// OriginalFilename
						GetVersionStr(pVerData, szPath, "OriginalFilename", langcp.wCodePage,
							ver.sOriginalFilename);

						// ProductName
						GetVersionStr(pVerData, szPath, "ProductName", langcp.wCodePage,
							ver.sProductName);

						// ProductVersion
						GetVersionStr(pVerData, szPath, "ProductVersion", langcp.wCodePage,
							ver.sProductVersion);
					}
				}
			}

			delete[] pVerData;
		}

		return result;
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	// public methods





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods

}