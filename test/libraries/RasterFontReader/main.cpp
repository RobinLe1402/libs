#include "rl/lib/RasterFontReader.hpp"
#include <memory>
#include <Windows.h>

inline void ShowUsage();
inline void ShowUnknownExt();

int wmain(int argc, wchar_t* argv[])
{
	if (argc < 2)
	{
		ShowUsage();
		return 1;
	}

	const wchar_t* const szPath = argv[1];

	const DWORD dwAttribs = GetFileAttributesW(szPath);
	if (dwAttribs == INVALID_FILE_ATTRIBUTES)
	{
		printf("File \"%ls\" couldn't be read.\n", szPath);
		return 1;
	}
	if (dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
	{
		ShowUsage();
		return 1;
	}

	const wchar_t* szFileExt = szPath + wcslen(szPath) - 1;
	while (szFileExt != szPath && szFileExt[0] != '.' && szFileExt[0] != '\\' &&
		szFileExt[0] != '/')
		--szFileExt;

	if (szFileExt[0] != '.')
	{
		ShowUnknownExt();
		return 1;
	}
	++szFileExt; // skip "."

	const size_t lenExt = wcslen(szFileExt);
	auto up_szExtUpper = std::make_unique<wchar_t[]>(lenExt + 1);
	auto szExtUpper = up_szExtUpper.get();
	szExtUpper[lenExt] = 0;
	for (size_t iChar = 0; iChar < lenExt; ++iChar)
	{
		wchar_t c = szFileExt[iChar];
		// check if digit or letter
		if (c < '0' || (c > '9' && c < 'A') || (c > 'Z' && c < 'a') || c > 'z')
		{
			ShowUnknownExt();
			return 1;
		}

		if (c >= 'a')
			szExtUpper[iChar] = c - ('a' - 'A');
		else
			szExtUpper[iChar] = c;
	}



	rl::RasterFontReader::RasterFontFaceCollection faces;

	if (wcscmp(szExtUpper, L"CPI") == 0)
	{
		using res = rl::RasterFontReader::LoadResult_CPI;
		const auto result = faces.loadFromFile_CPI(szPath);
		
		switch (result)
		{
		case res::FileNotOpened:
			printf("File couldn't be opened.\n");
			return 1;
		case res::InternalError:
			printf("Internal error while reading.\n");
			return 1;
		case res::UnexpectedEOF:
			printf("Unexpected EOF.\n");
			return 1;
		case res::UnexpectedValue:
			printf("Unexpected value in file.\n");
			return 1;
		}
	}
	else if (wcscmp(szExtUpper, L"FON") == 0)
	{
		using res = rl::RasterFontReader::LoadResult_FON;
		const auto result = faces.loadFromFile_FON(szPath);

		switch (result)
		{
		case res::FileNotOpened:
			printf("File couldn't be opened.\n");
			return 1;
		case res::InternalError:
			printf("Internal error while reading.\n");
			return 1;
		case res::UnexpectedEOF:
			printf("Unexpected EOF.\n");
			return 1;
		}
	}
	else if (wcscmp(szExtUpper, L"FNT") == 0)
	{
		printf("This kind of font file cannot be tested.\n"
			"Please use CPI or FON files instead.\n");
		return 1;
	}

	printf("Faces: %llu\n", faces.fontfaces().size());
	for (auto& face : faces.fontfaces())
	{
		auto& meta = face.meta();
		printf("  CP = %04d, Height = %02d, Name = \"%s\"\n",
			meta.iCodepage, meta.iHeight, meta.sFaceName.c_str());
	}

	return 0;
}



void ShowUsage()
{
	printf("Usage: RasterFontReader_Test Filename\n"
		"  Filename     Path of a CPI or FON file\n");
}

void ShowUnknownExt()
{
	printf("File extension must either be \".CPI\" or \".FON\".\n");
}
