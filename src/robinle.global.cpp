#include <rl/robinle.global.hpp>

// RobinLe
#include <rl/data.registry.settings.hpp>

// Win32
#include <Windows.h>



namespace rl
{
	namespace global
	{
		bool Initialize()
		{
			constexpr char szINITIALIZATION_ERROR[]     = "RobinLe initialization error";
			constexpr wchar_t wszINITIALIZATION_ERROR[] = L"RobinLe initialization error";

			if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS))
			{
				MessageBoxA(NULL, "Couldn't add custom DLL paths.", szINITIALIZATION_ERROR,
				MB_ICONERROR | MB_SYSTEMMODAL);
				return false;
			}

			HKEY hKey;
			std::wstring sDLLPath;
			if (rl::Registry::OpenRLKey(rl::Registry::RLKey::User, true, hKey))
			{
				const auto oVal = rl::Registry::GetValue(hKey, szREGISTRY_VALUE_DLLDIR);
				if (oVal.tryGetString(sDLLPath, true))
					goto lbStringFound;
			}
			if (rl::Registry::OpenRLKey(rl::Registry::RLKey::Global, true, hKey))
			{
				const auto oVal = rl::Registry::GetValue(hKey, szREGISTRY_VALUE_DLLDIR);
				if (oVal.tryGetString(sDLLPath, true))
					goto lbStringFound;
			}

			MessageBoxA(NULL, "No RobinLe DLL path was defined.\nPlease execute rlConfig.",
				szINITIALIZATION_ERROR, MB_ICONERROR | MB_SYSTEMMODAL);
			return false;

		lbStringFound:
			if (!SetDllDirectoryW(sDLLPath.c_str()))
			{
				std::wstring sMessage = L"Path \"";
				sMessage += sDLLPath;
				sMessage += L"\" couldn't be added to the DLL directories.";
				MessageBoxW(NULL, sMessage.c_str(), wszINITIALIZATION_ERROR,
					MB_ICONERROR | MB_SYSTEMMODAL);
				return false;
			}
			return true;
		}
	}
}