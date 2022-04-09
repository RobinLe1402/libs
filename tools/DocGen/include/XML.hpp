#pragma once
#ifndef ROBINLE_DOCGEN_XML
#define ROBINLE_DOCGEN_XML





#include <map>
#include <string>
#include <vector>





// The name given to pseudo-nodes that are actually only text.
// Reason: A node might look like this: "<xml>Hello<br/>World</xml>"
//         So how do you put that into a node structure?
//         Like this:
//           <xml>
//             <#text>Hello</#text>
//             <br/>
//             <#text>World!</#text>
//           </xml>
constexpr wchar_t szXMLNodeName_Text[] = L"#text";



class XMLNode
{
public: // methods

	XMLNode();
	XMLNode(const XMLNode& other);
	XMLNode(XMLNode&& rval) noexcept;
	~XMLNode() = default;

	/// <summary>
	/// Parse an XML node
	/// </summary>
	/// <param name="szText">XML of only the node to parse</param>
	/// <param name="bRequireHeader"></param>
	/// <returns>
	/// If the parsing failed, 0<para />
	/// If the parsing succeeded, the count of characters read
	/// </returns>
	size_t parse(const wchar_t* const szText);

	/// <summary>
	/// Clear all data associated with this node
	/// </summary>
	void clear();

	auto& name() const { return m_sName; }
	auto& params() const { return m_oParams; }
	auto& childNodes() const { return m_oChildNodes; }
	auto isTextNode() const { return m_bTextNode; }
	auto& text() const { return m_sValue; }


private: // methods

	size_t parseInternal(const wchar_t* const szText, bool bRequireHeader);

	size_t parseHeader(const wchar_t* szText, bool& bVal);
	size_t parseContents(const wchar_t* szText);
	size_t parseAsTextNode(const wchar_t* szText);


private: // variables

	bool m_bData;

	std::wstring m_sName;
	std::map<std::wstring, std::wstring> m_oParams;
	bool m_bTextNode;
	std::wstring m_sValue;
	std::vector<XMLNode> m_oChildNodes;
};





#endif // ROBINLE_DOCGEN_XML
