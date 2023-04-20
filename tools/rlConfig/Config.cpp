#include "Config.hpp"

// RobinLe
#include <rl/data.registry.settings.hpp>
#include <rl/robinle.global.hpp>


bool Config::load(bool bGlobal)
{
	m_sAppDir.clear();
	m_sDLLDir.clear();
	
	const rl::Registry::RLKey eKey = bGlobal ?
		rl::Registry::RLKey::Global : rl::Registry::RLKey::User;

	HKEY hKey;
	if (!rl::Registry::OpenRLKey(eKey, true, hKey))
		return false;
	rl::UniqueRegistryKey urk = hKey;

	const auto oAppDir = rl::Registry::GetValue(hKey, rl::global::szREGISTRY_VALUE_APPDIR);
	if (!oAppDir.tryGetString(m_sAppDir, true))
		m_sAppDir.clear();

	const auto oDLLDir = rl::Registry::GetValue(hKey, rl::global::szREGISTRY_VALUE_DLLDIR);
	if (!oDLLDir.tryGetString(m_sDLLDir, true))
		m_sDLLDir.clear();

	return true;
}

bool Config::save(bool bGlobal)
{
	const rl::Registry::RLKey eKey = bGlobal ?
		rl::Registry::RLKey::Global : rl::Registry::RLKey::User;

	HKEY hKey;
	if (!rl::Registry::OpenRLKey(eKey, false, hKey))
		return false;
	rl::UniqueRegistryKey urk = hKey;

	bool bResult = true;
	if (!m_sAppDir.empty() && !rl::Registry::SetValue(hKey, rl::global::szREGISTRY_VALUE_APPDIR,
		rl::RegistryValue::ByExpandString(m_sAppDir.c_str())))
		bResult = false;
	if (!m_sDLLDir.empty() && !rl::Registry::SetValue(hKey, rl::global::szREGISTRY_VALUE_DLLDIR,
		rl::RegistryValue::ByExpandString(m_sDLLDir.c_str())))
		bResult = false;

	return bResult;
}

bool Config::deleteUser()
{
	HKEY hKey;
	if (!rl::Registry::OpenRLKey(rl::Registry::RLKey::User, false, hKey))
		return false;
	rl::UniqueRegistryKey urk = hKey;
	
	return
		RegDeleteValueW(hKey, rl::global::szREGISTRY_VALUE_APPDIR) == ERROR_SUCCESS &&
		RegDeleteValueW(hKey, rl::global::szREGISTRY_VALUE_DLLDIR) == ERROR_SUCCESS;
}

bool Config::setAppDir(const wchar_t *szAppDir) noexcept
{
	if (szAppDir == nullptr)
		return false;

	if (szAppDir[0] == 0)
	{
		m_sAppDir.clear();
		return true;
	}

	const DWORD dwAttr = GetFileAttributesW(szAppDir);
	if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
	{
		m_sAppDir = szAppDir;
		return true;
	}
	return false;
}

bool Config::setDLLDir(const wchar_t *szDLLDir) noexcept
{
	if (szDLLDir == nullptr)
		return false;

	if (szDLLDir[0] == 0)
	{
		m_sDLLDir.clear();
		return true;
	}

	const DWORD dwAttr = GetFileAttributesW(szDLLDir);
	if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
	{
		m_sDLLDir = szDLLDir;
		return true;
	}
	return false;
}
