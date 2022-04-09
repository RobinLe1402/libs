#pragma once
#ifndef ROBINLE_DOCGEN_GENERATOR
#define ROBINLE_DOCGEN_GENERATOR





class DocumentationGenerator
{
public: // methods

	DocumentationGenerator() = default;
	~DocumentationGenerator();
	
	bool open(const wchar_t* szGitPath);

	bool generateHTML(const wchar_t* szDestFolder);

	void close();

	inline bool isOpened() { return m_bOpened; }


private: // variables

	bool m_bOpened = false;

};





#endif // ROBINLE_DOCGEN_GENERATOR