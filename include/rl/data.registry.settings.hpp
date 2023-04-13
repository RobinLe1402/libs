/***************************************************************************************************
 FILE:	data.registry.settings.hpp
 CPP:	data.registry.settings.cpp
 DESCR:	Interface to the RobinLe registry infrastructure.
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DATA_REGISTRY_SETTINGS
#define ROBINLE_DATA_REGISTRY_SETTINGS





#include "data.registry.hpp"

// STL
#include <map>



//==================================================================================================
// DECLARATION
namespace rl
{

	class SettingsRegistryKey final
	{
	public: // methods

		auto &values() noexcept { return m_oValues; }
		const auto &values() const noexcept { return m_oValues; }

		auto &subKeys() noexcept { return m_oSubKeys; }
		const auto &subKeys() const noexcept { return m_oSubKeys; }

		void clear();
		
	private: // variables

		std::map<std::wstring, RegistryValue> m_oValues;
		std::map<std::wstring, SettingsRegistryKey> m_oSubKeys;

	};





	class AppSettings final
	{
	public: // methods

		AppSettings(const wchar_t *szAppName = nullptr);
		~AppSettings() = default;
		AppSettings(const AppSettings &) = default;
		AppSettings(AppSettings &&) = default;

		AppSettings &operator=(const AppSettings &) = default;
		AppSettings &operator=(AppSettings &&) = default;


		bool load();
		bool save(bool bClearBeforeWriting = true);

		auto &appName() noexcept { return m_sAppName; }
		const auto &appName() const noexcept { return m_sAppName; }

		auto &rootKey() noexcept { return m_oRootKey; }
		const auto rootKey() const noexcept { return m_oRootKey; }


	private: // variables

		std::wstring m_sAppName;
		SettingsRegistryKey m_oRootKey;

	};



	class GlobalSettings final
	{
	public: // methods
		
		bool load();
		bool save(bool bClearBeforeWriting = true);

		auto &rootKey() noexcept { return m_oRootKey; }
		const auto &rootKey() const noexcept { return m_oRootKey; }


	private: // variables

		SettingsRegistryKey m_oRootKey;

	};
	
}





#endif // ROBINLE_DATA_REGISTRY_SETTINGS