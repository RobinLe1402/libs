#include "include/XML.hpp"
#include <regex>


// RegEx for an XML opening tag
// Matches
//   0..1
// Groups:
//   1: Node name
//   2: Parameter text (all parameters concatenated, as found in the string)
//   3: (optional) node closing slash ("/")
const wchar_t szRegEx_XML_OpeningTag[] =
LR"~(^(?:<(\w+)(?:\s*((?:\w+(?:="[^"<>]*")?\s*?)*))\s*(?:(\/)>|>)){1}\s*)~";

// RegEx for XML opening tag parameters
// Matches
//   1..n
// Groups:
//   1: Parameter name
//   2: (optional) parameter value
const wchar_t szRegEx_XML_Parameters[] =
LR"~((\w+)(?:="([^"]*)")?)~";

// RegEx for checking if a node is a textnode
// Matches
//   0..1
// Groups:
//   1: Text value (including linebreaks)
const wchar_t szRegEx_XML_Text[] =
LR"~(^\s*([^<>]+)\s*)~";





XMLNode::XMLNode() :
	m_bData(false),
	m_bTextNode(false)
{}

XMLNode::XMLNode(const XMLNode& other) :
	m_bData(other.m_bData),
	m_sName(other.m_sName),
	m_oParams(other.m_oParams),
	m_bTextNode(other.m_bTextNode),
	m_sValue(other.m_sValue),
	m_oChildNodes(other.m_oChildNodes)
{ }

XMLNode::XMLNode(XMLNode&& rval) noexcept :
	m_bData(rval.m_bData),
	m_sName(std::move(rval.m_sName)),
	m_oParams(std::move(rval.m_oParams)),
	m_bTextNode(rval.m_bTextNode),
	m_sValue(std::move(rval.m_sValue)),
	m_oChildNodes(std::move(rval.m_oChildNodes))
{
	rval.m_bData = false;
	rval.m_bTextNode = false;
}

size_t XMLNode::parse(const wchar_t* const szText)
{
	// skip whitespace at start of string
	size_t iWhitespace = 0;
	{
		while (szText[iWhitespace] != 0 && isspace(szText[iWhitespace]))
			++iWhitespace;
	}

	const size_t iParsed = parseInternal(szText + iWhitespace, true);
	if (iParsed == 0)
		return 0;
	else
	{
		m_bData = true;
		return iWhitespace + iParsed;
	}
}

void XMLNode::clear()
{
	if (!m_bData)
		return;

	m_sName.clear();
	m_oParams.clear();
	m_bTextNode = false;
	m_sValue.clear();
	m_oChildNodes.clear();
}

size_t XMLNode::parseInternal(const wchar_t* const szText, bool bRequireHeader)
{
	clear();

	bool bVal{};
	const size_t iHeaderLength = parseHeader(szText, bVal);
	if (iHeaderLength == 0)
	{
		size_t iTextLen = parseAsTextNode(szText);

		if (iTextLen == 0)
		{
			clear();
			return 0;
		}
		else
			return iTextLen;
	}
	if (!bVal)
		return iHeaderLength;

	const size_t iContentLength = parseContents(szText + iHeaderLength);
	if (iContentLength == 0)
	{
		clear();
		return 0;
	}

	return iHeaderLength + iContentLength;
}

size_t XMLNode::parseHeader(const wchar_t* szText, bool& bVal)
{
	clear();

	std::wregex regex(szRegEx_XML_OpeningTag);
	std::wcmatch oCResults;
	std::wsmatch oSResults;

	if (!std::regex_search(szText, oCResults, regex))
		return 0;

	m_sName = oCResults[1];

	const auto& sParams = oCResults[2].str();
	if (!sParams.empty())
	{
		regex = szRegEx_XML_Parameters;
		auto it = sParams.cbegin();
		while (std::regex_search(it, sParams.cend(), oSResults, regex))
		{
			if (oSResults.size() == 3)
				m_oParams.emplace(oSResults[1].str(), oSResults[2].str());
			else
				m_oParams.emplace(oSResults[1].str(), L"");

			it = oSResults.suffix().first;
		}
	}
	if (m_sName == L"br") // "<br>" never has a "</br>"
		bVal = false;
	else
		bVal = !oCResults[3].matched;
	return oCResults.length();
}

size_t XMLNode::parseContents(const wchar_t* szText)
{
	size_t iOffsetAbs = 0;
	size_t iOffsetRel = 0;
	do
	{
		XMLNode oSubNode;
		iOffsetRel = oSubNode.parseInternal(szText + iOffsetAbs, false);

		if (iOffsetRel > 0)
		{
			m_oChildNodes.push_back(std::move(oSubNode));
			iOffsetAbs += iOffsetRel;
		}
	} while (iOffsetRel > 0);

	if (szText[iOffsetAbs] == 0)
		return 0; // this node was not closed

	std::wstring sRegEx_EndTag;
	sRegEx_EndTag.reserve(m_sName.length() + 3);
	sRegEx_EndTag += L"</" + m_sName + L'>';

	std::wstring_view sRemainingText(szText + iOffsetAbs);

	if (!sRemainingText.starts_with(sRegEx_EndTag))
		return 0; // ill-formed closing tag

	iOffsetAbs += sRegEx_EndTag.length();

	// skip whitespace
	while (szText[iOffsetAbs] != 0 && isspace(szText[iOffsetAbs]))
		++iOffsetAbs;

	// a single child node that is also a text pseudo node
	// --> transfer text to this node, delete child
	if (m_oChildNodes.size() == 1 && m_oChildNodes[0].isTextNode() &&
		m_oChildNodes[0].name() == szXMLNodeName_Text)
	{
		m_sValue = std::move(m_oChildNodes[0].m_sValue);
		m_oChildNodes.clear();
		m_bTextNode = true;
	}

	return iOffsetAbs;
}

size_t XMLNode::parseAsTextNode(const wchar_t* szText)
{
	std::wregex oRegEx(szRegEx_XML_Text);
	std::wcmatch oMatch;
	if (!std::regex_search(szText, oMatch, oRegEx) || !oMatch[1].matched || oMatch.position() != 0)
		return 0;

	m_sName = szXMLNodeName_Text;
	m_bTextNode = true;

	m_sValue.reserve(oMatch[1].str().length());
	std::wregex oRegExWhitespace(LR"~(\s+)~");

	const std::wstring sText(oMatch[1].str());

	std::regex_replace(std::back_inserter(m_sValue), sText.cbegin(), sText.cend(), oRegExWhitespace,
		L" ");

	if (m_sValue.ends_with(' '))
		m_sValue.pop_back();

	m_sValue.shrink_to_fit(); // release unrequired memory

	return oMatch.length();
}
