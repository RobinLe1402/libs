#include "XMLParser.hpp"

namespace
{
	
	enum class XMLTokenType
	{
		NodeBeginChar,  // '<'
		NodeEndChar,    // '>'
		NodeCloseChar,  // '/'
		AssignmentChar, // '='
		Text,           // text outside of quotes
		String          // text insidoe of quotes (escape sequences not resolved yet)
	};

	class XMLToken
	{
	public: // methods

		XMLToken(XMLTokenType eType) : m_eType(eType) {}
		virtual ~XMLToken() = default;

		XMLTokenType type() const { return m_eType; }


	private: // variables

		XMLTokenType m_eType;

	};


	class XMLToken_NodeBeginChar : public XMLToken
	{
	public: // methods
		XMLToken_NodeBeginChar() : XMLToken(XMLTokenType::NodeBeginChar) {}
	};

	class XMLToken_NodeEndChar : public XMLToken
	{
	public: // methods
		XMLToken_NodeEndChar() : XMLToken(XMLTokenType::NodeEndChar) {}
	};

	class XMLToken_NodeCloseChar : public XMLToken
	{
	public: // methods
		XMLToken_NodeCloseChar() : XMLToken(XMLTokenType::NodeCloseChar) {}
	};

	class XMLToken_AssignmentChar : public XMLToken
	{
	public: // methods
		XMLToken_AssignmentChar() : XMLToken(XMLTokenType::AssignmentChar) {}
	};



	class XMLTextToken : public XMLToken
	{
	public: // methods
		XMLTextToken(XMLTokenType eType, const std::string &sText) :
			XMLToken(eType), m_sText(sText) {}
		XMLTextToken(XMLTokenType eType, std::string &&sText) :
			XMLToken(eType), m_sText(std::move(sText)) {}

		const std::string &text() const { return m_sText; }

	private: // variables

		std::string m_sText;
	};


	class XMLToken_Text : public XMLTextToken
	{
	public: // methods
		XMLToken_Text(const std::string &sIdent) :
			XMLTextToken(XMLTokenType::Text, sIdent) {}
	};

	class XMLToken_String : public XMLTextToken
	{
	public: // methods
		XMLToken_String(const std::string &sString) :
			XMLTextToken(XMLTokenType::String, sString) {}
	};




	bool IsXMLIdent(const std::string &s)
	{
		if (s.length() == 0)
			return false;

		char c = s[0];
		if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && (c != '_'))
			return false;

		for (size_t i = 1; i < s.length(); ++i)
		{
			c = s[i];

			if (c & 0x80)
				return false;

			if (!std::isalnum(c) && c != '-' && c != '_' && c != '.')
				return false;
		}

		return true;
	}


	std::string IntermediateGetText(const std::vector<std::shared_ptr<XMLToken>> &oTokens,
		size_t iStartToken, size_t &iLastReadToken)
	{
		std::string sVal;

		for (iLastReadToken = iStartToken; iLastReadToken < oTokens.size(); ++iLastReadToken)
		{
			switch (oTokens[iLastReadToken]->type())
			{
			case XMLTokenType::AssignmentChar:
				if (sVal.length() > 0)
					sVal += ' ';
				sVal += '=';
				break;

			case XMLTokenType::NodeCloseChar:
				if (sVal.length() > 0)
					sVal += ' ';
				sVal += '/';
				break;

			case XMLTokenType::Text:
				if (sVal.length() > 0)
					sVal += ' ';
				sVal += static_cast<const XMLToken_Text *>(oTokens[iLastReadToken].get())->text();
				break;

			default:
				--iLastReadToken;
				return sVal;
			}
		}

		return sVal;
	}

	bool IntermediateToNode(
		const std::vector<std::shared_ptr<XMLToken>> &oTokens, size_t iStartToken,
		XMLNode &oDestNode, size_t &iLastReadToken)
	{
		if (oTokens.size() <= iStartToken)
			return false;

		const size_t iRemainingTokens = oTokens.size() - iStartToken;
		if (iRemainingTokens < 4)
			return false;

		if (oTokens[iStartToken]->type() != XMLTokenType::NodeBeginChar)
			return false;
		const auto &oIdent = *oTokens[iStartToken + 1];
		if (oIdent.type() != XMLTokenType::Text)
			return false;
		const XMLToken_Text &oIdentText =
			*static_cast<XMLToken_Text *>(oTokens[iStartToken + 1].get());

		if (!IsXMLIdent(oIdentText.text()))
			return false;

		oDestNode.name() = oIdentText.text();

		// read node "opener"
		bool bOpenerRead = false;
		for (iLastReadToken = iStartToken + 2; iLastReadToken < oTokens.size(); ++iLastReadToken)
		{
			const auto &oToken = *oTokens[iLastReadToken];

			switch (oToken.type())
			{
			case XMLTokenType::Text:
			{
				const auto &oAttribToken =
					*static_cast<const XMLToken_Text *>(oTokens[iLastReadToken].get());
				const auto &sPropertyName = oAttribToken.text();

				if (!IsXMLIdent(sPropertyName))
					return false;

				++iLastReadToken;
				if (iLastReadToken >= oTokens.size())
					return false;
				if (oTokens[iLastReadToken]->type() != XMLTokenType::AssignmentChar)
				{
					oDestNode.attributes()[sPropertyName] = "";

					--iLastReadToken;
					break;
				}

				++iLastReadToken;
				if (iLastReadToken >= oTokens.size())
					return false;
				if (oTokens[iLastReadToken]->type() != XMLTokenType::String)
					return false;
				const auto &oAttribValToken =
					*static_cast<const XMLToken_String *>(oTokens[iLastReadToken].get());
				oDestNode.attributes()[sPropertyName] = oAttribValToken.text();

				break;
			}

			case XMLTokenType::NodeCloseChar: // self-closing node
				++iLastReadToken;
				if (iLastReadToken < oTokens.size() &&
					oTokens[iLastReadToken]->type() == XMLTokenType::NodeEndChar)
					return true;
				else
					return false;



			case XMLTokenType::NodeEndChar: // regular node (seperate ending)
				++iLastReadToken;
				bOpenerRead = true;
				break;



			default:
				return false;
			}

			if (bOpenerRead)
				break;
		}
		if (!bOpenerRead)
			return false;

		
		// read subnodes + closer
		bool bCloserRead = false;
		for (; iLastReadToken < oTokens.size(); ++iLastReadToken)
		{
			const auto &oToken = *oTokens[iLastReadToken];
			switch (oToken.type())
			{
			case XMLTokenType::NodeBeginChar:
			{
				if (iLastReadToken + 1 < oTokens.size())
				{
					if (oTokens[iLastReadToken + 1]->type() == XMLTokenType::NodeCloseChar)
					{
						if (iLastReadToken + 3 >= oTokens.size())
							return false; // would require name and '>'

						if (oTokens[iLastReadToken + 2]->type() == XMLTokenType::Text &&
							static_cast<const XMLToken_Text *>(
								oTokens[iLastReadToken + 2].get())->text() == oDestNode.name() &&
							oTokens[iLastReadToken + 3]->type() == XMLTokenType::NodeEndChar)
						{
							iLastReadToken += 3;
							bCloserRead = true;
							break;
						}
						else
							return false; // invalid XML
					}
				}

				XMLNode oChildNode;
				if (!IntermediateToNode(oTokens, iLastReadToken, oChildNode, iLastReadToken))
					return false;

				oDestNode.children().push_back(std::move(oChildNode));
				break;
			}

			case XMLTokenType::AssignmentChar:
			case XMLTokenType::NodeEndChar:
			case XMLTokenType::Text:
			{
				XMLNode oTextNode;
				oTextNode.name() = szNODENAME_TEXT;
				oTextNode.attributes()[szATTRIBNAME_TEXTVAL] =
					IntermediateGetText(oTokens, iLastReadToken, iLastReadToken);
				oDestNode.children().push_back(std::move(oTextNode));
			}
				break;

			default:
				return false;
			}

			if (bCloserRead)
				break;
		}
		if (!bCloserRead)
			return false;

		return true;
	}

}





void XMLNode::clear()
{
	m_sName.clear();
	m_oAttributes.clear();
	m_oChildNodes.clear();
}

std::string XMLNode::textValue() const
{
	if (m_oChildNodes.size() != 1 || m_oChildNodes[0].name() != szNODENAME_TEXT)
		return "";

	return m_oChildNodes[0].attributes().at(szATTRIBNAME_TEXTVAL);
}

bool XMLDoc::loadFromString(const char *szXML)
{
	m_oRootNode.clear();
	
	std::vector<std::shared_ptr<XMLToken>> oIntermediate;
	const char *sz = szXML;

	// convert to intermediate
	while (*sz != 0)
	{
		switch (*sz)
		{
		case '<':
			// comment
			if (*(sz + 1) != 0 && (*(sz + 1) == '!') &&
				*(sz + 2) != 0 && (*(sz + 2) == '-') &&
				*(sz + 3) != 0 && (*(sz + 3) == '-'))
			{
				sz += 3;
				while (*sz != 0)
				{
					if (*sz == '-' &&
						*(sz + 1) != 0 && *(sz + 1) == '-' &&
						*(sz + 2) != 0 && *(sz + 2) == '>')
					{
						sz += 3;
						break; // comment
					}
					++sz;
				}
			}
			else
			{
				oIntermediate.push_back(std::make_shared<XMLToken_NodeBeginChar>());
				++sz;
			}

			break;

		case '>':
			oIntermediate.push_back(std::make_shared<XMLToken_NodeEndChar>());
			++sz;
			break;

		case '/':
			oIntermediate.push_back(std::make_shared<XMLToken_NodeCloseChar>());
			++sz;
			break;

		case '=':
			oIntermediate.push_back(std::make_shared<XMLToken_AssignmentChar>());
			++sz;
			break;

		case ' ':
		case '\r':
		case '\n':
		case '\t':
			++sz;
			break; // do nothing

		case '"': // --> String
		{
			size_t iStringLen = 0;
			const char *pPreview = sz + 1;
			while (*pPreview != 0 && *pPreview != '"')
			{
				++iStringLen;
				++pPreview;
			}
			if (*pPreview != '"')
				return false; // no closing quotes

			std::string sString;
			sString.resize(iStringLen);
			for (size_t i = 0; i < iStringLen; ++i)
			{
				sString[i] = sz[i + 1];
			}
			oIntermediate.push_back(std::make_shared<XMLToken_String>(std::move(sString)));

			sz += iStringLen + 2;
		}
			break;

		default: // text
		{
			size_t iTextLen = 0;
			const char *pPreview = sz;
			while (*pPreview != 0 &&
				((*pPreview & 0x80) != 0 ||
					(!std::iscntrl(*pPreview) &&
					!std::isspace(*pPreview)) &&
					*pPreview != '=' &&
					*pPreview != '<' &&
					*pPreview != '>' &&
					*pPreview != '/'))
			{
				++iTextLen;
				++pPreview;
			}

			std::string sText;
			sText.resize(iTextLen);
			for (size_t i = 0; i < iTextLen; ++i)
				sText[i] = sz[i];

			oIntermediate.push_back(std::make_shared<XMLToken_Text>(std::move(sText)));

			sz += iTextLen;
		}
		}
	}



	// check syntax

	size_t iLastToken = 0;
	return IntermediateToNode(oIntermediate, 0, m_oRootNode, iLastToken) &&
		iLastToken == oIntermediate.size() - 1;

}

void XMLDoc::resolveTexts()
{
	resolveTextsInNode(m_oRootNode);
}

void XMLDoc::resolveTextsInNode(XMLNode &oNode)
{
	// 1. resolve escape sequences
	for (size_t i = 0; i < oNode.children().size(); ++i)
	{
		auto &oChild = oNode.children()[i];
		if (oChild.name() == szNODENAME_TEXT)
		{
			oChild.attributes()[szATTRIBNAME_TEXTVAL] =
				XMLResolveEscapeSequences(oChild.attributes()[szATTRIBNAME_TEXTVAL]);
		}
	}



	// 2. resolve linebreaks
	for (size_t i = 0; i < oNode.children().size(); ++i)
	{
		auto &oChild = oNode.children()[i];
		if (oChild.name() == "br")
		{
			oChild.name()                             = szNODENAME_TEXT;
			oChild.attributes()[szATTRIBNAME_TEXTVAL] = "\n";
		}
	}



	// 3. concatenate texts
	for (size_t i = 0; i < oNode.children().size(); ++i)
	{
		auto &oChild = oNode.children()[i];
		if (oChild.name() == szNODENAME_TEXT)
		{
			while (++i < oNode.children().size())
			{
				if (oNode.children()[i].name() == szNODENAME_TEXT)
				{
					oChild.attributes()[szATTRIBNAME_TEXTVAL] =
						oChild.attributes()[szATTRIBNAME_TEXTVAL] +
						oNode.children()[i].attributes()[szATTRIBNAME_TEXTVAL];
					oNode.children().erase(oNode.children().begin() + i);
					--i;
				}
				else
					break; // while
			}
		}
	}



	// recursively resolve all child node texts
	for (auto &oChildNode : oNode.children())
	{
		if (oChildNode.name() != szNODENAME_TEXT)
			resolveTextsInNode(oChildNode);
	}
}



std::string XMLResolveEscapeSequences(const std::string &s)
{
	std::string sResult;
	sResult.reserve(s.length());

	for (size_t i = 0; i < s.length(); ++i)
	{
		if (s[i] != '&')
		{
			sResult += s[i];
			continue;
		}

		const auto iSemicolon = s.find(';', i + 1);
		if (iSemicolon == std::string::npos)
		{
			// no semicolon after this ampersand --> don't change anything
			sResult += '&';
			continue;
		}

		std::string sEscapeSeq = s.substr(i + 1, (iSemicolon - (i + 1)));

		// unicode: not supported (ASCII strings used)
		/*if (sEscapeSeq.starts_with("#"))
		{
			if (sEscapeSeq.starts_with("#x"))
			{
				sEscapeSeq = sEscapeSeq.substr(2);
				auto i = std::stoi(sEscapeSeq, nullptr, 16);
				sResult += 
			}
		}*/

		if (sEscapeSeq == "amp")
			sResult += '&';
		else if (sEscapeSeq == "apos")
			sResult += '\'';
		else if (sEscapeSeq == "gt")
			sResult += '>';
		else if (sEscapeSeq == "lt")
			sResult += '<';
		else if (sEscapeSeq == "quot")
			sResult += '\"';

		i = iSemicolon;

	}

	sResult.shrink_to_fit();
	return sResult;
}
