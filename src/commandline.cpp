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

		std::wregex regex{LR"REGEX(\/(\w+):(.*))REGEX"};
		std::wsmatch matches;
		if (!std::regex_search(m_sRaw, matches, regex))
		{
			m_sName.clear();
			m_sNameUpper.clear();
			m_sValue.clear();
		}
		else
		{
			m_sName = matches[1].str();
			m_sNameUpper = m_sName;
			std::locale loc;
			for (size_t i = 0; i < m_sNameUpper.length(); ++i)
			{
				m_sNameUpper[i] = std::toupper(m_sNameUpper[i], loc);
			}

			m_sValue = matches[2].str();
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
