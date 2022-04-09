#include "include/Generator.hpp"


DocumentationGenerator::~DocumentationGenerator() { close(); }

bool DocumentationGenerator::open(const wchar_t* szGitPath)
{
	close();

	return false; // ToDo:
}

bool DocumentationGenerator::generateHTML(const wchar_t* szDestFolder)
{
	if (!m_bOpened)
		return false;

	return false; // ToDo:
}

void DocumentationGenerator::close()
{
	if (!m_bOpened)
		return;

	// ToDo:
}
