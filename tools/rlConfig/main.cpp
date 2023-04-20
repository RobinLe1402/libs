// Project
#include "Config.hpp"

// RobinLe
#include <rl/robinle.global.hpp>
#include <rl/commandline.hpp>
#include <rl/runasadmin.hpp>

// STL
#include <algorithm>
#include <cstdio>
#include <cwctype>
#include <iostream>
#include <string>

// Win32
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

constexpr wchar_t szARG_MODE[] = L"MODE";
constexpr wchar_t szARG_MODE_VAL_GLOBAL[] = L"GLOBAL";
constexpr wchar_t szARG_MODE_VAL_USER[]   = L"USER";

constexpr wchar_t szTITLE_MODESELECT[] = L"rlConfig | Choose mode";
constexpr wchar_t szTITLE_MAIN_USER[]   = L"rlConfig (User Mode)";
constexpr wchar_t szTITLE_MAIN_GLOBAL[] = L"rlConfig (Global Mode)";

char GetUserChoice(const char *szOptionsUpper, const wchar_t *szMessage);

std::wstring GetPathFromUser(const std::wstring &sPrevValue, const wchar_t *szValDescr);

int wmain(int argc, wchar_t *argv[])
{
	SetConsoleTitleA("rlConfig");

	Config cfg;
	const bool bGlobalExists = cfg.load(true);
	const bool bLocalExists  = cfg.load(false);
	if (bGlobalExists && bLocalExists)
	{
		const char cInput = GetUserChoice("NY",
			L"There is both a global and a user configuration.\n"
			L"Should the user configuration be deleted? [Y/N] ");

		if (cInput == 'Y')
		{
			if (!cfg.deleteUser())
			{
				std::printf("Error: Deletion failed.\n");
				return 1;
			}
			std::printf("Successfully deleted user configuration.\n");
			return 0;
		}
	}

	bool bGlobal;
	bool bModeSetByArgs = false;

	{
		auto &cmd = rl::Commandline::Instance();
		auto it = cmd.findNamedValue(szARG_MODE, false);
		if (it != cmd.end())
		{
			bModeSetByArgs = true;

			std::wstring sVal = it->value();
			try
			{
				std::transform(sVal.begin(), sVal.end(), sVal.begin(), std::towupper);


				if (sVal == szARG_MODE_VAL_USER)
					bGlobal = false;
				else if (sVal == szARG_MODE_VAL_GLOBAL)
					bGlobal = true;
				else
					bModeSetByArgs = false;
			}
			catch (const std::exception &e)
			{
				std::printf("Error evaluating commandline: \"%s\"\n", e.what());
				bModeSetByArgs = false;
			}
		}
	}


	if (!bModeSetByArgs)
	{
		SetConsoleTitleW(szTITLE_MODESELECT);

		const char cInput = GetUserChoice("GU",
			L"Which configuration would you like to edit?\n"
			L"  [G]lobal (requires elevation)\n"
			L"  [U]ser\n"
			L"  ");

		bGlobal = cInput == 'G';
	}



	if (bGlobal)
	{
		auto &raa = rl::RunAsAdmin::Instance();

		if (!raa.isAdmin())
		{
			std::wstring sArgs = L"/";
			sArgs += szARG_MODE;
			sArgs += L':';
			sArgs += szARG_MODE_VAL_GLOBAL;
			for (int i = 1; i < argc; ++i)
			{
				sArgs.reserve(sArgs.length() + wcslen(argv[i]) + 3);

				sArgs += L" \"";
				sArgs += argv[i];
				sArgs += L'\"';
			}

			const auto eResult = raa.elevateSelf(sArgs.c_str());

			switch (eResult)
			{
			case rl::ElevationResult::Success:
				return 0;

			default:
				std::cerr << "Couldn't elevate self.\n";
				return 1;
			}
		}
	}





	system("CLS");

	if (bGlobal)
		SetConsoleTitleW(szTITLE_MAIN_GLOBAL);
	else
		SetConsoleTitleW(szTITLE_MAIN_USER);

	cfg.load(bGlobal); // return value ignored because irrelevant

	cfg.setAppDir(GetPathFromUser(cfg.appDir(),
		L"RobinLe application path").c_str());
	std::printf("\n");
	cfg.setDLLDir(GetPathFromUser(cfg.dllDir(),
		L"RobinLe DLL path").c_str());
	std::printf("\n");

	if (!cfg.save(bGlobal))
	{
		std::printf("Error trying to save configuration.\n");
		return 1;
	}
	std::printf("Configuration was successfully changed.\n");


	return 0;
}



char GetUserChoice(const char *szOptionsUpper, const wchar_t *szMessage)
{
	const std::string_view svOptionsUpper = szOptionsUpper;

	while (true)
	{
		system("CLS");
		std::printf("%ls", szMessage);

		std::string sInput;
		std::getline(std::cin, sInput);

		if (sInput.length() != 1)
			continue; // not only a single character

		char cInput = sInput[0];
		if (cInput & 0x80)
			continue; // not ASCII
		if (std::islower(cInput))
			cInput = std::toupper(cInput);

		for (char c : svOptionsUpper)
		{
			if (cInput == c)
				return c;
		}
	}
}

std::wstring GetPathFromUser(const std::wstring &sPrevValue, const wchar_t *szValDescr)
{
	std::printf("%ls\n", szValDescr);
	if (!sPrevValue.empty())
		std::printf("  Current value: \"%ls\"\n", sPrevValue.c_str());
	else
		std::printf("  <Not set yet>\n");

	std::wstring sInput;
	while (true)
	{
		std::printf("  New value: ");
		std::getline(std::wcin, sInput);
		if (sInput.empty() && !sPrevValue.empty())
			return sPrevValue;

		wchar_t szExpanded[MAX_PATH + 1];
		if (!ExpandEnvironmentStringsW(sInput.c_str(), szExpanded, MAX_PATH + 1))
		{
			std::cerr << "  Error: Couldn't process path. Please enter a different one.\n\n";
			continue;
		}

		const DWORD dwAttributes = GetFileAttributesW(szExpanded);
		if (dwAttributes == INVALID_FILE_ATTRIBUTES || !(dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			std::cerr <<
				"  Error: Invalid path. Make sure to enter the path of an existing directory.\n\n";
		}
		else
		{
			if (PathIsRelativeW(szExpanded))
			{
				std::cerr << "  Error: Please enter an absolute path, not a relative one.\n\n";
				continue;
			}

			return sInput;
		}
	}
}
