#include <rl/data.registry.settings.hpp>


namespace
{

	const std::wstring &GetAppName()
	{
		static std::wstring s_sAppName;
		if (!s_sAppName.empty())
			return s_sAppName;

		wchar_t szPath[MAX_PATH + 1];

		if (GetModuleFileNameW(NULL, szPath, MAX_PATH + 1) == 0)
			return s_sAppName;

		std::wstring_view sv = szPath;
		auto iLastDelim = sv.find_last_of('\\');
		if (iLastDelim != sv.npos)
			sv = sv.substr(iLastDelim + 1);
		iLastDelim = sv.find_last_of('.');
		if (iLastDelim != sv.npos)
			sv = sv.substr(0, iLastDelim);

		s_sAppName = sv;
		return s_sAppName;
	}

	const HKEY hBaseKey_Apps = HKEY_CURRENT_USER;
	constexpr wchar_t szBasePath_Apps[] =
		LR"(Software\RobinLe\Apps)";

	const HKEY hBaseKey_Global = HKEY_LOCAL_MACHINE;
	constexpr wchar_t szBasePath_Global[] =
		LR"(SOFTWARE\RobinLe)";


	class HKeyHelper final
	{
	public:

		HKeyHelper(HKEY hKey) : m_hKey(hKey) {}
		~HKeyHelper() { RegCloseKey(m_hKey); }


	private:

		HKEY m_hKey;

	};


	bool LoadSettings(HKEY hBaseKey, const wchar_t *szSubPath, rl::SettingsRegistryKey &oDest)
	{
		HKEY hKey;
		if (RegOpenKeyExW(hBaseKey, szSubPath, 0, KEY_ENUMERATE_SUB_KEYS | KEY_READ,
			&hKey) != ERROR_SUCCESS)
			return false;

		HKeyHelper oKeyHelper(hKey);

		DWORD dwSubKeys;
		DWORD dwMaxSubKeyNameLen;
		DWORD dwValues;
		DWORD dwMaxValueNameLen;
		if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &dwSubKeys, &dwMaxSubKeyNameLen, NULL,
			&dwValues, &dwMaxValueNameLen, NULL, NULL, NULL) != ERROR_SUCCESS)
			return false;

		// SUBKEYS ---------------------------------------------------------------------------------
		if (dwSubKeys > 0)
		{
			auto upKeyName = std::make_unique<wchar_t[]>((size_t)dwMaxSubKeyNameLen + 1);

			DWORD dwIndex = 0;
			DWORD dwBufLen = dwMaxSubKeyNameLen + 1;
			while (RegEnumKeyExW(hKey, dwIndex, upKeyName.get(), &dwBufLen, NULL, NULL, NULL,
				NULL) != ERROR_NO_MORE_ITEMS)
			{
				rl::SettingsRegistryKey oKey;
				if (LoadSettings(hKey, upKeyName.get(), oKey))
				{
					oDest.subKeys().emplace(upKeyName.get(), std::move(oKey));
				}

				++dwIndex;
				dwBufLen = dwMaxSubKeyNameLen + 1;
			}
		}


		// VALUES ----------------------------------------------------------------------------------
		if (dwValues > 0)
		{
			auto upValueName = std::make_unique<wchar_t[]>((size_t)dwMaxValueNameLen + 1);

			DWORD dwIndex = 0;
			DWORD dwNameBufLen = dwMaxValueNameLen + 1;
			while (RegEnumValueW(hKey, dwIndex, upValueName.get(), &dwNameBufLen, NULL, NULL, NULL,
				NULL) != ERROR_NO_MORE_ITEMS)
			{
				rl::RegistryValue oVal = rl::Registry::GetValue(hKey, upValueName.get());
				if (oVal)
					oDest.values().emplace(upValueName.get(), std::move(oVal));

				dwNameBufLen = dwMaxValueNameLen + 1;
				++dwIndex;
			}
		}



		return true;

	}

	bool SaveSettings(HKEY hBaseKey, const wchar_t *szSubPath, const rl::SettingsRegistryKey &oSrc,
		bool bClearBeforeWriting)
	{
		constexpr REGSAM samDesired = KEY_ALL_ACCESS;

		if (bClearBeforeWriting)
		{
			const auto result = RegDeleteTreeW(hBaseKey, szSubPath);

			switch (result)
			{
			case ERROR_FILE_NOT_FOUND:
			case ERROR_SUCCESS:
				break;

			default:
				return false;
			}
		}


		HKEY hKey = NULL;
		switch (RegOpenKeyExW(hBaseKey, szSubPath, 0, samDesired, &hKey))
		{
		case ERROR_SUCCESS:
			break;

		case ERROR_FILE_NOT_FOUND:
			if (RegCreateKeyExW(hBaseKey, szSubPath, NULL, NULL, REG_OPTION_NON_VOLATILE,
				samDesired, NULL, &hKey, NULL) != ERROR_SUCCESS)
				return false;
			break;

		default:
			return false;
		}

		HKeyHelper oKeyHelper(hKey);

		for (const auto &it : oSrc.values())
		{
			if (!rl::Registry::SetValue(hKey, it.first.c_str(), it.second))
				return false;
		}

		for (const auto &it : oSrc.subKeys())
		{
			HKEY hSubKey;
			if (RegCreateKeyExW(hKey, it.first.c_str(), NULL, NULL, REG_OPTION_NON_VOLATILE,
				samDesired, NULL, &hSubKey, NULL) != ERROR_SUCCESS)
				return false;
			RegCloseKey(hSubKey);

			if (!SaveSettings(hKey, it.first.c_str(), it.second, false))
				return false;
		}

		return true;
	}

}


namespace rl
{

	void SettingsRegistryKey::clear()
	{
		m_oValues.clear();
		m_oSubKeys.clear();
	}



	AppSettings::AppSettings(const wchar_t *szAppName)
	{
		if (szAppName == nullptr)
			szAppName = GetAppName().c_str();

		m_sAppName = szAppName;
	}

	bool AppSettings::load()
	{
		m_oRootKey.clear();

		std::wstring sSubKey = szBasePath_Apps;
		sSubKey += L'\\' + m_sAppName;

		return LoadSettings(hBaseKey_Apps, sSubKey.c_str(), m_oRootKey);
	}

	bool AppSettings::save(bool bClearBeforeWriting)
	{
		std::wstring sSubKey = szBasePath_Apps;
		sSubKey += L'\\' + m_sAppName;

		return SaveSettings(hBaseKey_Apps, sSubKey.c_str(), m_oRootKey, bClearBeforeWriting);
	}



	bool GlobalSettings::load()
	{
		m_oRootKey.clear();
		return LoadSettings(hBaseKey_Global, szBasePath_Global, m_oRootKey);
	}

	bool GlobalSettings::save(bool bClearBeforeWriting)
	{
		return SaveSettings(hBaseKey_Global, szBasePath_Global, m_oRootKey, bClearBeforeWriting);
	}

}
