/***************************************************************************************************
 FILE:	commandline.hpp
 CPP:	commandline.cpp
 DESCR:	Helper singleton for working with command-line arguments.
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_COMMANDLINE
#define ROBINLE_COMMANDLINE





//==================================================================================================
// INCLUDES

#include <cstdint>
#include <string>
#include <vector>



//==================================================================================================
// DECLARATION
namespace rl
{

	class CommandlineArgument
	{
	public: // methods

		void initialize(const wchar_t *szRawArg);

		auto &raw() const noexcept { return m_sRaw; }
		auto &name() const noexcept { return m_sName; }
		auto &nameUppercase() const noexcept { return m_sNameUpper; }
		auto &value() const noexcept { return m_sValue; }


	private: // variables

		std::wstring m_sRaw;
		std::wstring m_sName;
		std::wstring m_sNameUpper;
		std::wstring m_sValue;

	};

	class Commandline
	{
	public: // types

		using iterator = std::vector<CommandlineArgument>::const_iterator;
		using reverse_iterator = std::vector<CommandlineArgument>::const_reverse_iterator;


	public: // static methods

		static Commandline &Instance() noexcept { return s_oInstance; }


	private: // static variables

		static Commandline s_oInstance;


	public: // methods

		auto size() const noexcept { return m_oArgs.size(); }
		const CommandlineArgument &operator[](size_t iIndex) const { return m_oArgs.at(iIndex); }

		iterator begin() const noexcept { return m_oArgs.begin(); }
		iterator end() const noexcept { return m_oArgs.end(); }
		reverse_iterator rbegin() const noexcept { return m_oArgs.rbegin(); }
		reverse_iterator rend() const noexcept { return m_oArgs.rend(); }

		iterator find(const wchar_t *szArgName, bool bCaseSensitive, size_t iStartArg = 0) const;


	private: // methods

		Commandline();
		~Commandline() = default;


	private: // variables

		std::vector<CommandlineArgument> m_oArgs;

	};

}





#endif // ROBINLE_COMMANDLINE