#pragma once
#ifndef ROBINLE_UNIT_LIB_GEN_SOURCEGENERATOR
#define ROBINLE_UNIT_LIB_GEN_SOURCEGENERATOR





#include <rl/text.fileio.hpp>

#include <string>
#include <vector>



class SourceFile
{
public: // methods

	bool open(const wchar_t *szPath); // ATTENTION! Doesn't save already included units!
	void close();
	bool save();

	auto &units() const { return m_oFilenames; }
	auto &units() { return m_oFilenames; }


private: // types

	enum class Type
	{
		Project,
		Filters
	};


private: // variables

	bool m_bOpened = false;

	std::wstring m_sFilePath;
	rl::TextFileInfo m_oEncoding{};
	Type m_eType;

	std::vector<std::wstring> m_oPrefix;
	std::vector<std::wstring> m_oSuffix;

	std::wstring m_sIndent;

	std::vector<std::wstring> m_oFilenames; // unit names, e.g. "main.cpp" or "$(EnvVar)\test.cpp"


};





#endif // ROBINLE_UNIT_LIB_GEN_SOURCEGENERATOR