#include <rl/dll/UnicodeData.hpp>
#include <cstdio> // printf

int main(int argc, char* argv[])
{
	rl::UnicodeDataDLL::UChar_t iVal = u'€';
	auto len = rl::UnicodeDataDLL::GetNameLen(iVal);
	if (len == 0)
		printf("Codepoint U+%04X has no name", iVal);
	else
	{
		char* szName = new char[len];
		auto written = rl::UnicodeDataDLL::GetName(iVal, szName, len);

		printf("Codepoint U+%04X has the following name:\n\"%s\"", iVal, szName);

		delete[] szName;
	}

	printf("\n\n");

	return 0;
}
