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

	/// <summary>
	/// A settings node.<para />
	/// May contain subnodes and values.
	/// </summary>
	class SettingsNode final
	{
	public: // methods

		auto &values() noexcept { return m_oValues; }
		const auto &values() const noexcept { return m_oValues; }

		auto &subNodes() noexcept { return m_oSubKeys; }
		const auto &subNodes() const noexcept { return m_oSubKeys; }

		void clear();
		
	private: // variables

		std::map<std::wstring, RegistryValue> m_oValues;
		std::map<std::wstring, SettingsNode> m_oSubKeys;

	};





	/// <summary>
	/// App-specific settings.<para />
	/// Are always user-specific.
	/// </summary>
	class AppSettings final
	{
	public: // methods

		AppSettings(const wchar_t *szAppName = nullptr);
		~AppSettings() = default;
		AppSettings(const AppSettings &) = default;
		AppSettings(AppSettings &&) = default;

		AppSettings &operator=(const AppSettings &) = default;
		AppSettings &operator=(AppSettings &&) = default;



		/// <summary>
		/// Set the app name.<para />
		/// The name cannot contain any "<c>\</c>" or "<c>/</c>".
		/// </summary>
		/// <returns>Was the app name updated?</returns>
		bool setAppName(const wchar_t *szAppName) noexcept;
		/// <summary>
		/// The app name used for loading and saving the settings.<para />
		/// By default, this value is set to the EXE filename without a file extension.
		/// If no app name can be generated, the string "<c>[UnnamedApp]</c>" is used.
		/// </summary>
		const auto &appName() const noexcept { return m_sAppName; }

		/// <summary>
		/// Try to load the app-specific settings of the app named <c>appName()</c> from the
		/// registry.
		/// </summary>
		/// <returns>Could settings be loaded?</returns>
		bool load();
		/// <summary>
		/// Try to save the app-specific settings of the app named <c>appName()</c> to the registry.
		/// </summary>
		/// <para name="bClearBeforeWriting">
		/// Should the previous settings be deleted?
		/// </para>
		/// <returns>Could the settings be saved?</returns>
		bool save(bool bClearBeforeWriting = true);

		auto &rootKey() noexcept { return m_oRootKey; }
		const auto rootKey() const noexcept { return m_oRootKey; }


	private: // variables

		std::wstring m_sAppName;
		SettingsNode m_oRootKey;

	};



	/// <summary>
	/// Global RobinLe settings.<para />
	/// Are always system-wide.
	/// </summary>
	class GlobalSettings final
	{
	public: // methods
		
		/// <summary>
		/// Try to load the global settings from the registry.
		/// </summary>
		/// <returns>Could settings be loaded?</returns>
		bool load();
		/// <summary>
		/// Try to save the global settings from the registry.
		/// </summary>
		/// <returns>Could the settings be saved?</returns>
		bool save(bool bClearBeforeWriting = true);

		auto &rootKey() noexcept { return m_oRootKey; }
		const auto &rootKey() const noexcept { return m_oRootKey; }


	private: // variables

		SettingsNode m_oRootKey;

	};
	
}





#endif // ROBINLE_DATA_REGISTRY_SETTINGS