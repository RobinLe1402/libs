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

	/// <summary>The different types of arguments.</summary>
	enum class CommandlineArgumentType
	{
		/// <summary>
		/// Informal argument. Only the raw text is set.<para/>
		/// Example: "<c>TestArg</c>"
		/// </summary>
		Text,

		/// <summary>
		/// A flag without an associated value. Raw text and name are set.<para/>
		/// Example: "<c>/TestArg</c>"
		/// </summary>
		Flag,

		/// <summary>
		/// Informal argument. Raw text, name and value are set.<para/>
		/// Example: "<c>/TestArg:TestVal</c>"<para/>
		/// The value might still be empty.
		/// </summary>
		NamedVal
	};

	/// <summary>A commandline argument.</summary>
	class CommandlineArgument
	{
	public: // methods

		void initialize(const wchar_t *szRawArg);

		auto type() const noexcept { return m_eType; }

		/// <summary>The raw text value of the argument.</summary>
		auto &raw() const noexcept { return m_sRaw; }
		/// <summary>The name of the argument.</summary>
		/// <remarks>Consists only of ASCII letters, numbers and underscore.</remarks>
		auto &name() const noexcept { return m_sName; }
		/// <summary>The name of the argument, in all uppercase.</summary>
		/// <remarks>Uppercase version of <c>name()</c>.</remarks>
		auto &nameUppercase() const noexcept { return m_sNameUpper; }
		/// <summary>The value of the argument.</summary>
		auto &value() const noexcept { return m_sValue; }


	private: // variables

		CommandlineArgumentType m_eType = CommandlineArgumentType::Text;
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

		/// <summary>Search for a flag/named value.</summary>
		iterator find(const wchar_t *szArgName, bool bCaseSensitive, size_t iStartArg = 0) const;
		/// <summary>Search for a flag.</summary>
		iterator findFlag(const wchar_t *szName, bool bCaseSensitive, size_t iStartArg = 0) const;
		/// <summary>Search for a named value.</summary>
		iterator findNamedValue(const wchar_t *szName, bool bCaseSensitive, size_t iStartArg = 0)
			const;


	private: // methods

		Commandline();
		~Commandline() = default;

		iterator findArgInternal(const wchar_t *szArgName, bool bCaseSensitive, size_t iStartArg,
			bool bFlag, bool bNamedValue) const noexcept;


	private: // variables

		std::vector<CommandlineArgument> m_oArgs;

	};

}





#endif // ROBINLE_COMMANDLINE