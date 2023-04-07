#include <rl/runasadmin.hpp>

// Win32
#include <Windows.h>

// STL
#include <cstdint>
#include <string>



namespace rl
{



	RunAsAdmin::RunAsAdmin()
	{
		HANDLE hToken = NULL;

		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		{
			TOKEN_ELEVATION tkElevation{};
			DWORD cbSize = sizeof(TOKEN_ELEVATION);
			if (GetTokenInformation(hToken, TokenElevation, &tkElevation, sizeof(tkElevation),
				&cbSize))
			{
				m_bAdmin = tkElevation.TokenIsElevated != 0;
			}
		}
	}

	ElevationResult RunAsAdmin::elevateSelf(const wchar_t *szArgs, const wchar_t *szCurrentDir)
	{
		if (isAdmin())
			return ElevationResult::AlreadyElevated; // already admin

		wchar_t szPath[MAX_PATH + 1];
		if (GetModuleFileNameW(NULL, szPath, MAX_PATH + 1) == 0)
			return ElevationResult::Failed;


		return execute(szPath, nullptr, nullptr);
	}

	ElevationResult RunAsAdmin::execute(const wchar_t *szExe, const wchar_t *szArgs,
		const wchar_t *szCurrentDir)
	{
		std::wstring sArgs;
		if (szArgs != nullptr && szArgs[0] != '\0')
			sArgs = szArgs;
		else
		{
			sArgs.reserve(wcslen(GetCommandLineW()));

			int  argc = 0;
			auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
			for (int i = 1; i < argc; ++i)
			{
				sArgs += L" \"";
				sArgs += argv[i];
				sArgs += L"\"";
			}
		}


		std::wstring sCurrentDir;
		if (szCurrentDir != nullptr && szCurrentDir[0] != '\0')
			sCurrentDir = szCurrentDir;
		else
		{
			wchar_t szCurrentDir[MAX_PATH + 1];
			if (GetCurrentDirectoryW(MAX_PATH + 1, szCurrentDir) == 0)
				return ElevationResult::Failed;
			sCurrentDir = szCurrentDir;
		}

		const auto result = ShellExecuteW(NULL, L"runas", szExe, sArgs.c_str(), sCurrentDir.c_str(),
			SW_SHOW);

		if (reinterpret_cast<intptr_t>(result) > 32)
			return ElevationResult::Success;
		else
		{
			switch (reinterpret_cast<intptr_t>(result))
			{
			case SE_ERR_ACCESSDENIED:
				return ElevationResult::Denied;

			default:
				return ElevationResult::Failed;
			}
		}
	}

	ElevationResult ElevateSelf(bool bExitIfFailed, bool bErrorDialog)
	{
		RunAsAdmin o;

		if (o.isAdmin())
			return ElevationResult::AlreadyElevated; // already admin

		const auto eResult = o.elevateSelf();

		switch (eResult)
		{
		case ElevationResult::Success:
		case ElevationResult::AlreadyElevated:
			return eResult;

		case ElevationResult::Denied:
			if (bErrorDialog)
				MessageBoxA(NULL, "Failed to elevate process: Access denied", "Error",
											MB_OK | MB_ICONERROR);
			if (bExitIfFailed)
				exit(1);
			else
				return ElevationResult::Denied;

		case ElevationResult::Failed:
			if (bErrorDialog)
				MessageBoxA(NULL, "Failed to elevate process", "Error", MB_OK | MB_ICONERROR);
			if (bExitIfFailed)
				exit(1);

			break;
		}

		return ElevationResult::Failed;
	}



}
