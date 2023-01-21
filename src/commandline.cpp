#include <rl/commandline.hpp>

// STL
#include <locale>
#include <regex>

// Win32
#include <Windows.h>



namespace rl
{

	void CommandlineArgument::initialize(const wchar_t *szRawArg)
	{
		m_sRaw = szRawArg;
		m_sName.clear();
		m_sNameUpper.clear();
		m_sValue.clear();



		// named value
		std::wregex regex{LR"REGEX(^\/(\w+):(.*)$)REGEX"};
		std::wsmatch matches;
		if (std::regex_search(m_sRaw, matches, regex))
		{
			m_eType = CommandlineArgumentType::NamedVal;
			m_sName = matches[1].str();

			m_sValue = matches[2].str();
		}

		// flag/text
		else
		{
			// flag
			regex = LR"REGEX(^\/(\w+)$)REGEX";
			if (std::regex_search(m_sRaw, matches, regex))
			{
				m_eType = CommandlineArgumentType::Flag;
				m_sName = matches[1].str();
			}

			// text
			else
				m_eType = CommandlineArgumentType::Text;
		}


		// generate uppercase name
		m_sNameUpper = m_sName;
		if (!m_sNameUpper.empty())
		{
			std::locale loc;
			for (size_t i = 0; i < m_sNameUpper.length(); ++i)
			{
				m_sNameUpper[i] = std::toupper(m_sNameUpper[i], loc);
			}
		}
	}



	Commandline Commandline::s_oInstance;

	Commandline::Commandline()
	{
		auto szCommandline = GetCommandLineW();
		int argc = 0;
		auto argv = CommandLineToArgvW(szCommandline, &argc);

		for (size_t i = 0; i < argc; ++i)
		{
			CommandlineArgument oArg;
			oArg.initialize(argv[i]);
			m_oArgs.push_back(std::move(oArg));
		}
	}

	Commandline::iterator Commandline::find(const wchar_t *szArgName, bool bCaseSensitive,
		size_t iStartArg) const
	{
		return findArgInternal(szArgName, bCaseSensitive, iStartArg, true, true);
	}

	Commandline::iterator Commandline::findFlag(const wchar_t *szName, bool bCaseSensitive,
		size_t iStartArg) const
	{
		return findArgInternal(szName, bCaseSensitive, iStartArg, true, false);
	}

	Commandline::iterator Commandline::findNamedValue(const wchar_t *szName, bool bCaseSensitive,
		size_t iStartArg) const
	{
		return findArgInternal(szName, bCaseSensitive, iStartArg, false, true);
	}

	Commandline::iterator Commandline::findArgInternal(const wchar_t *szArgName,
		bool bCaseSensitive, size_t iStartArg, bool bFlag, bool bNamedValue) const noexcept
	{
		if (iStartArg >= size())
			return end();

		std::wstring sArgName = szArgName;
		if (!bCaseSensitive)
		{
			std::locale loc;
			for (size_t i = 0; i < sArgName.length(); ++i)
			{
				sArgName[i] = std::toupper(sArgName[i], loc);
			}
		}

		for (auto it = begin() + iStartArg; it != end(); ++it)
		{
			switch (it->type())
			{
			case CommandlineArgumentType::Text:
				continue;
			case CommandlineArgumentType::Flag:
				if (bFlag)
					break;
				continue;
			case CommandlineArgumentType::NamedVal:
				if (bNamedValue)
					break;
				continue;
			}


			if (bCaseSensitive)
			{
				if (it->name() == sArgName)
					return it;
			}
			else
			{
				if (it->nameUppercase() == sArgName)
					return it;
			}
		}

		return end();
	}

}
