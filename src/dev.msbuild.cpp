#include <rl/dev.msbuild.hpp>

#include <rl/text.fileio.hpp>

#include <regex>
#include <Windows.h>



namespace
{
	std::wstring GetLogPath()
	{
		wchar_t szLogFile[MAX_PATH + 1]{};
		ExpandEnvironmentStringsW(LR"(%TEMP%\rlCompilerLog.txt)", szLogFile, MAX_PATH + 1);
		
		return szLogFile;
	}

	bool ParseCompileMessage(const wchar_t *szLine, rl::CompileMessage &msg,
		std::wstring *psProject)
	{
		using namespace rl;

		constexpr wchar_t szRegEx[] =
			LR"(^(.*)\((\d+),(\d+)\): (error|warning) ([A-Z]\d{4}|): (.+) \[(.+\.vcxproj)\]$)";

		msg = {};

		std::wregex oRegEx(szRegEx);
		std::wcmatch matches;
		if (!std::regex_search(szLine, matches, oRegEx) || !matches.ready())
			return false;

		msg.sFile   = matches.str(1);
		msg.iLine   = std::stoull(matches.str(2));
		msg.iColumn = std::stoull(matches.str(3));
		if (matches.str(4) == L"warning")
			msg.eType = CompileMessageType::Warning;
		else
			msg.eType = CompileMessageType::Error;

		std::wstring sCode = matches.str(5);
		if (!sCode.empty())
			msg.sCode = matches.str(5);

		msg.sDescription = matches.str(6);

		if (psProject)
			*psProject = matches.str(7);

		return true;
	}

}

namespace rl
{

	size_t ProjectCompileResult::warningCount() const
	{
		size_t iResult = 0;

		for (const auto &msg : m_oMessages)
		{
			if (msg.eType == CompileMessageType::Warning)
				++iResult;
		}

		return iResult;
	}

	size_t ProjectCompileResult::errorCount() const
	{
		size_t iResult = 0;

		for (const auto &msg : m_oMessages)
		{
			if (msg.eType == CompileMessageType::Error)
				++iResult;
		}

		return iResult;
	}

	bool ProjectCompileResult::warnings() const
	{
		for (const auto &msg : m_oMessages)
		{
			if (msg.eType == CompileMessageType::Warning)
				return true;
		}

		return false;
	}

	bool ProjectCompileResult::errors() const
	{
		for (const auto &msg : m_oMessages)
		{
			if (msg.eType == CompileMessageType::Error)
				return true;
		}

		return false;
	}



	CompileResult::CompileResult(const char *szErrorMessage)
		:
		m_bValid(false), m_sErrorMessage(szErrorMessage) { }



	const std::wstring Compiler::s_LogPath = GetLogPath();

	const std::wstring &Compiler::VSPath()
	{
		static std::wstring sPath;

		if (!sPath.empty())
			return sPath;

		constexpr wchar_t szVsWhereEXE_EnvVar[] =
			LR"(%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe)";
		wchar_t szVsWhereEXE_Resolved[MAX_PATH + 1]{};
		ExpandEnvironmentStringsW(szVsWhereEXE_EnvVar, szVsWhereEXE_Resolved, MAX_PATH + 1);

		if (GetFileAttributesW(szVsWhereEXE_Resolved) == INVALID_FILE_ATTRIBUTES)
			return sPath;

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
			return sPath;
		oReader.readLine(sPath);
		oReader.close();
		DeleteFileW(szTempFile_Resolved);

		return sPath;
	}

	const std::wstring &Compiler::Path()
	{
		static std::wstring sPath = VSPath();

		if (sPath.empty())
			return sPath;

		static bool bInit = false;

		if (!bInit)
		{
			sPath += LR"(\MSBuild\Current\Bin\MSBuild.exe)";
			bInit = true;
		}


		return sPath;
	}

	CompileResult Compiler::Compile(const wchar_t *szProject,
		const wchar_t *szPlatform, const wchar_t *szConfiguration)
	{
		std::wstring sCompilerPath = Path();

		if (sCompilerPath.empty())
			return CompileResult("Couldn't read compiler path");

		std::wstring sCommandLine = L"\"";
		sCommandLine += szProject;
		sCommandLine += L"\" -t:Rebuild -p:Configuration=";
		sCommandLine += szConfiguration;
		sCommandLine += L" -p:Platform=";
		sCommandLine += szPlatform;
		sCommandLine += LR"( -fileLogger -fileLoggerParameters:Verbosity=q;Encoding=Unicode;)"
			L"LogFile=\"";
		sCommandLine += s_LogPath;
		sCommandLine += L'\"';

		SHELLEXECUTEINFOW ShExecInfo ={ sizeof(SHELLEXECUTEINFOW) };
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = NULL;
		ShExecInfo.lpFile = sCompilerPath.c_str();
		ShExecInfo.lpParameters = sCommandLine.c_str();
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_HIDE;
		ShExecInfo.hInstApp = NULL;

		if (!ShellExecuteExW(&ShExecInfo))
			return CompileResult("Call to compiler failed");
		if (ShExecInfo.hProcess != NULL)
		{
			const bool bSuccess =
				WaitForSingleObject(ShExecInfo.hProcess, INFINITE) == WAIT_FAILED;
			CloseHandle(ShExecInfo.hProcess);
		}
		std::vector<std::wstring> oLogFile;
		if (!rl::ReadAllLines(s_LogPath.c_str(), oLogFile))
			return CompileResult("Couldn't open compiler log file");
		DeleteFileW(s_LogPath.c_str());


		CompileResult result;
		
		for (const auto &sLine : oLogFile)
		{
			CompileMessage cm;
			std::wstring sProject;

			if (!ParseCompileMessage(sLine.c_str(), cm, &sProject))
				continue;

			result.projects()[std::move(sProject)].items().push_back(std::move(cm));
		}

		return result;
	}



}
