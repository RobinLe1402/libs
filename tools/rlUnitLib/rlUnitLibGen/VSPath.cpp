#include "VSPath.hpp"

#include <rl/text.fileio.hpp>

#include <string>
#include <Windows.h>


// %ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe -latest -property installationPath
const wchar_t *GetVSInstallDir()
{
	static bool    bRead = false;
	static wchar_t szPath[MAX_PATH + 1]{};

	if (bRead)
		return szPath;

	constexpr wchar_t szVsWhereEXE_EnvVar[] =
		LR"(%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe)";
	wchar_t szVsWhereEXE_Resolved[MAX_PATH + 1]{};
	ExpandEnvironmentStringsW(szVsWhereEXE_EnvVar, szVsWhereEXE_Resolved, MAX_PATH + 1);

	if (GetFileAttributesW(szVsWhereEXE_Resolved) == INVALID_FILE_ATTRIBUTES)
		return nullptr;

	constexpr wchar_t szTempFile_EnvVar[] = LR"(%TEMP%\VSPath.txt)";
	wchar_t szTempFile_Resolved[MAX_PATH + 1]{};
	ExpandEnvironmentStringsW(szTempFile_EnvVar, szTempFile_Resolved, MAX_PATH + 1);

	constexpr wchar_t szParams[] = L"-latest -property installationPath";
	std::wstring sCommand =
		(std::wstring)L"\"" + szVsWhereEXE_Resolved + L"\" " + szParams +
		L" > " + szTempFile_Resolved;

	system("CHCP 65001 > NUL"); // UTF-8
	_wsystem(sCommand.c_str());

	rl::TextFileReader oReader(szTempFile_Resolved);
	if (!oReader)
		return nullptr;
	std::wstring sPath;
	oReader.readLine(sPath);
	wcscpy_s(szPath, sPath.c_str());
	oReader.close();
	DeleteFileW(szTempFile_Resolved);

	bRead = true;

	return szPath;
}
