#pragma once
#ifndef RLCONFIG_CONFIG
#define RLCONFIG_CONFIG



#include <string>



class Config final
{
public: // methods

	bool load(bool bGlobal);
	bool save(bool bGlobal);

	bool deleteUser();


	const auto &appDir() const noexcept { return m_sAppDir; }
	bool setAppDir(const wchar_t *szAppDir) noexcept;

	const auto &dllDir() const noexcept { return m_sDLLDir; }
	bool setDLLDir(const wchar_t *szDLLDir) noexcept;


private: // variables

	std::wstring m_sAppDir;
	std::wstring m_sDLLDir;

};



#endif // RLCONFIG_CONFIG