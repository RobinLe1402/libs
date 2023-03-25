#include "SourceGenerator.hpp"



namespace
{
	constexpr wchar_t szAutoGenBegin[] = L"<!--BEGIN AUTOGEN-->";
	constexpr wchar_t szAutoGenEnd[]   = L"<!--END AUTOGEN-->";
	
	constexpr wchar_t szProjectPrefix[] = LR"(<ClCompile Include=")";
	constexpr wchar_t szProjectSuffix[] = LR"(" />)";

	constexpr wchar_t szFiltersLine1Prefix[] = LR"(<ClCompile Include=")";
	constexpr wchar_t szFiltersLine1Suffix[] = LR"(">)";
	constexpr wchar_t szFiltersLine2[]       = LR"(  <Filter>Source Files</Filter>)";
	constexpr wchar_t szFiltersLine3[]       = LR"(</ClCompile>)";
}


bool SourceFile::open(const wchar_t *szPath)
{
	close();

	std::wstring_view svPath = szPath;
	if (svPath.ends_with(L".vcxproj"))
		m_eType = Type::Project;
	else if (svPath.ends_with(L".vcxproj.filters"))
		m_eType = Type::Filters;
	else
		return false;

	std::vector<std::wstring> oLines;
	{
		rl::TextFileReader oReader;
		oReader.open(szPath);
		if (!oReader)
			return false;

		m_oEncoding = oReader.encoding();
		oReader.readLines(oLines);
		oReader.close();
	}

	m_bOpened   = true;
	m_sFilePath = szPath;

	// search for begin comment
	size_t iBeginLine  = 0;
	bool   bBeginFound = false;
	m_oPrefix.reserve(oLines.size());
	for (; iBeginLine < oLines.size(); ++iBeginLine)
	{
		m_oPrefix.push_back(std::move(oLines[iBeginLine]));
		const auto &sLine = m_oPrefix.back();

		// "trim" left
		size_t iSpaceCount = sLine.find_first_not_of(L" \t");
		if (iSpaceCount == std::wstring::npos)
			continue; // only whitespaces

		std::wstring_view svText = sLine.c_str() + iSpaceCount;
		if (svText.starts_with(szAutoGenBegin))
		{
			bBeginFound = true;
			m_sIndent = sLine.substr(0, iSpaceCount);
			break; // for
		}
	}
	if (!bBeginFound)
	{
		close();
		return false;
	}
	m_oPrefix.shrink_to_fit();



	// search for end comment
	size_t iEndLine  = iBeginLine + 1;
	bool   bEndFound = false;
	for (; iEndLine < oLines.size(); ++iEndLine)
	{
		auto &sLine = oLines[iEndLine];

		// "trim" left
		size_t iSpaceCount = sLine.find_first_not_of(L" \t");
		if (iSpaceCount == std::wstring::npos)
			continue; // only whitespaces

		std::wstring_view svText = sLine.c_str() + iSpaceCount;
		if (svText.starts_with(szAutoGenEnd))
		{
			bEndFound = true;
			m_oSuffix.push_back(std::move(sLine));
			break; // for
		}
	}
	if (!bEndFound)
	{
		close();
		return false;
	}

	// copy remaining lines
	m_oSuffix.reserve(oLines.size() - iEndLine);
	for (size_t i = iEndLine; i < oLines.size(); ++i)
	{
		m_oSuffix.push_back(std::move(oLines[i]));
	}
	m_oSuffix.shrink_to_fit();


	return true;
}

void SourceFile::close()
{
	if (!m_bOpened)
		return;

	m_bOpened = false;
	m_sFilePath.clear();
	m_oEncoding = {};
	m_oPrefix.clear();
	m_oSuffix.clear();
	m_sIndent.clear();
	m_oFilenames.clear();
}

bool SourceFile::save()
{
	if (!m_bOpened)
		return false;

	rl::TextFileWriter oWriter(m_sFilePath.c_str(), m_oEncoding);
	if (!oWriter)
		return false;

	oWriter.writeLines(m_oPrefix);

	// write actual contents
	for (const auto &sUnit : m_oFilenames)
	{
		switch (m_eType)
		{
		case Type::Project:
			oWriter.writeLine(m_sIndent + szProjectPrefix + sUnit + szProjectSuffix);
			break;

		case Type::Filters:
			oWriter.writeLine(m_sIndent + szFiltersLine1Prefix + sUnit + szFiltersLine1Suffix);
			oWriter.writeLine(m_sIndent + szFiltersLine2);
			oWriter.writeLine(m_sIndent + szFiltersLine3);
			break;
		}
	}

	oWriter.writeLines(m_oSuffix);
	oWriter.close();

	return true;
}
