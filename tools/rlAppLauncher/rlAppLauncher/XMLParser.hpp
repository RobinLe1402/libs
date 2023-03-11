#pragma once
#ifndef ROBINLE_APPLAUNCHER_XMLPARSER
#define ROBINLE_APPLAUNCHER_XMLPARSER





#include <map>
#include <memory>
#include <string>
#include <vector>


constexpr char szNODENAME_TEXT[] = "#text";
constexpr char szATTRIBNAME_TEXTVAL[] = "value";

class XMLNode
{
public: // methods

	std::string &name() { return m_sName; }
	const std::string &name() const { return m_sName; }

	auto &attributes() { return m_oAttributes; }
	const auto &attributes() const { return m_oAttributes; }

	auto &children() { return m_oChildNodes; }
	const auto &children() const { return m_oChildNodes; }

	void clear();

	std::string textValue() const;


private: // variables

	std::string m_sName;
	std::map<std::string, std::string> m_oAttributes;
	std::vector<XMLNode> m_oChildNodes;

};



class XMLDoc
{
public: // methods

	/// <summary>Load an ASCII XML document from a C-String.</summary>
	/// <returns>Could the XML be parsed successfully?</returns>
	bool loadFromString(const char *szXML);

	void resolveTexts();

	const auto &rootNode() const { return m_oRootNode; }


private: // methods

	void resolveTextsInNode(XMLNode &oNode);


private: // variables

	XMLNode m_oRootNode;

};


std::string XMLResolveEscapeSequences(const std::string &s);





#endif // ROBINLE_APPLAUNCHER_XMLPARSER