#include "!Include/rlFNTPROJ.hpp"

namespace rlFont = rl::rlFNTPROJ;

#include <chrono>
#include <fstream>
#include <Windows.h>



namespace
{

	const char szMAGICNO[] = "rlFONTPROJECT";

	struct FILE_Header
	{
		char     szMagicNo[14];
		uint16_t iVersionNo;
		uint64_t iFilesize;
		uint64_t iOffsetStringtable;
		uint64_t iStringCount;
		uint64_t stridxFoundry;
		uint64_t stridxDesigner;
		uint64_t stridxComments;
		int64_t  unixtimeCreated;
	};

	struct FILE_FontFamilyHeader
	{
		uint64_t stridxName;
		uint64_t stridxCopyright;
		char32_t cFallback;
		uint16_t iTypefaceClass;
		uint8_t  reserved[3];
		uint64_t iFontCount;
	};

	struct FILE_FontHeader
	{
		uint64_t stridxName;
		uint64_t stridxCopyright;
		char32_t cFallback;
		uint16_t iWeight;
		uint8_t  iFlags;
		uint8_t  reserved;
		uint64_t iFaceCount;
	};
	constexpr uint8_t
		FontFlag_Italic = 0x01,
		FontFlag_Underline = 0x02,
		FontFlag_Strikeout = 0x04;

	struct FILE_FontFaceHeader
	{
		uint64_t stridxName;
		uint64_t stridxCopyright;
		char32_t cFallback;
		uint16_t iXHeight;
		uint16_t iCapHeight;
		uint16_t iPoints;
		uint8_t  reserved[6];
		uint64_t iCharCount;
	};

	struct FILE_CharMeta
	{
		uint16_t iWidth;
		uint16_t iHeight;
		uint16_t iBaseline;
		uint16_t iPaddingLeft;
		uint16_t iPaddingRight;
		uint8_t  reserved[2];
	};

	struct FILE_CharTableEntry
	{
		char32_t ch;
		FILE_CharMeta oMetadata;
	};

}





// DEFINITIONS -----------------------------------------------------------------------------

rlFont::Char::Char(uint16_t iWidth, uint16_t iHeight, const CharMeta &oMetadata) :
	m_iWidth(iWidth), m_iHeight(iHeight),
	m_iBytesPerRow(iWidth % 8 ? (iWidth / 8) + 1 : iWidth / 8),
	m_iDataSize(m_iBytesPerRow *iHeight),
	m_oMetadata(oMetadata)
{
	if (iWidth == 0 || iHeight == 0)
		throw std::exception("Illegal character size");

	m_upData = std::make_unique<uint8_t[]>(m_iDataSize);
}

rlFont::Char::Char(const Char &other) :
	m_iWidth(other.m_iWidth), m_iHeight(other.m_iHeight),
	m_iBytesPerRow(other.m_iBytesPerRow),
	m_iDataSize(other.m_iDataSize),
	m_upData(std::make_unique<uint8_t[]>(other.m_iDataSize)),
	m_oMetadata(other.m_oMetadata)
{
	std::memcpy(m_upData.get(), other.m_upData.get(), other.m_iDataSize);
}

rlFont::Char &rlFont::Char::operator=(const Char &other)
{
	m_iWidth = other.m_iWidth;
	m_iHeight = other.m_iHeight;
	m_iBytesPerRow = other.m_iBytesPerRow;
	m_iDataSize = other.m_iDataSize;
	m_oMetadata = other.m_oMetadata;

	m_upData = std::make_unique<uint8_t[]>(m_iDataSize);
	std::memcpy(m_upData.get(), other.m_upData.get(), m_iDataSize);

	return *this;
}

void rlFont::Char::create(uint16_t iWidth, uint16_t iHeight)
{
	if (iWidth == 0 || iHeight == 0)
		throw std::exception("Illegal character size");

	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iBytesPerRow = (iWidth % 8) ? size_t(iWidth / 8) + 1 : iWidth / 8;
	m_iDataSize = m_iBytesPerRow * iHeight;
	m_upData = std::make_unique<uint8_t[]>(m_iDataSize);
}

void rlFont::Char::clear() noexcept
{
	m_iWidth = 0;
	m_iHeight = 0;
	m_iBytesPerRow = 0;
	m_iDataSize = 0;
	m_upData = nullptr;
	m_oMetadata ={};
}

bool rlFont::Char::getPixel(uint16_t iX, uint16_t iY) const
{
	if (iX >= m_iWidth || iY >= m_iHeight)
		throw std::exception("Invalid pixel position");

	return (m_upData.get()[iY * m_iBytesPerRow + (iX / 8)] >> (7 - iX % 8)) & 1;
}

void rlFont::Char::setPixel(uint16_t iX, uint16_t iY, bool bNewVal)
{
	if (iX >= m_iWidth || iY >= m_iHeight)
		throw std::exception("Invalid pixel position");

	uint8_t &iData = m_upData.get()[iY * m_iBytesPerRow + (iX / 8)];
	uint8_t iShiftedBit = 1 << (7 - iX % 8);
	iData &= ~iShiftedBit; // mask out bit
	if (bNewVal)
		iData |= iShiftedBit;
}

void rlFont::Char::setData(const uint8_t *pBuf, size_t lenBuf)
{
	if (lenBuf != m_iDataSize)
		throw std::exception("Buffer size doesn't match data size!");

	memcpy_s(m_upData.get(), m_iDataSize, pBuf, lenBuf);
}



rlFont::FontFace::FontFace(uint16_t iDefaultWidth, uint16_t iDefaultHeight,
	const CharMeta &oDefaultMetadata, const FontFaceMeta &oMetadata) :
	m_oMetadata(oMetadata),
	m_iDefaultCharWidth(iDefaultWidth),
	m_iDefaultCharHeight(iDefaultHeight),
	m_oDefaultCharMetadata(oDefaultMetadata)
{}

void rlFont::FontFace::clear() noexcept
{
	m_oChars.clear();
	m_oMetadata ={};

	m_iDefaultCharWidth = 0;
	m_iDefaultCharHeight = 0;
	m_oDefaultCharMetadata ={};
}



rlFont::Font::Font(const FontMeta &oMetadata) :
	m_oMetadata(oMetadata)
{}

void rlFont::Font::clear() noexcept
{
	m_oMetadata ={};
	m_oFaces.clear();
}



rlFont::FontFamily::FontFamily(const FontFamilyMeta &oMetadata) : m_oMeta(oMetadata) {}

void rlFont::FontFamily::clear() noexcept
{
	m_oFonts.clear();
	m_oMeta ={};
}



rlFont::Project::Project(const ProjectMeta &oMetadata, const FontFamilyMeta &oFFMeta) :
	m_oMeta(oMetadata), m_oFontFamily(oFFMeta) { }

void rlFont::Project::clear() noexcept
{
	m_oFontFamily.clear();
	m_oMeta ={};
	setCreationUNIXTimestamp();
}

bool rlFont::Project::loadFromFile(const wchar_t *szFilename)
{
	clear();

	std::ifstream file(szFilename, std::ios::binary);
	if (!file)
		return false; // couldn't open file for reading

	file.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);

	try
	{
#define READVAR(var) file.read(reinterpret_cast<char*>(&var), sizeof(var))
		// file header
		{
			file.seekg(0, std::ios::end);
			const size_t iFilesize = file.tellg();
			file.seekg(0, std::ios::beg);

			FILE_Header hdr{};
			READVAR(hdr);

			if (strcmp(hdr.szMagicNo, szMAGICNO) != 0)
				return false; // wrong magic number
			if (hdr.iFilesize != iFilesize)
				return false; // wrong file size

			switch (hdr.iVersionNo)
			{
			case 1:
				if (!loadFromFileV1(&file, &hdr))
					throw std::exception();
				break;
			default:
				return false; // unknown version number
			}
		}

#undef READVAR
	}
	catch (...)
	{
		clear();
		return false;
	}

	return true;
}

bool rlFont::Project::saveToFile(const wchar_t *szFilename) const
{
	std::ofstream file(szFilename, std::ios::binary);
	if (!file)
		return false; // couldn't open file for writing

	file.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);

#define WRITE(var) file.write(reinterpret_cast<const char*>(&var), sizeof(var))
#define MAKESTRIDX(strref)			\
	(strref.empty() ? 0 : (oStrings.push_back(&strref), oStrings.size()))
	try
	{
		std::vector<const std::wstring *> oStrings;

		// all dynamic data in the file header is written at this offset.
		std::streamoff iOffsetDynamicData;

		// file header
		{
			FILE_Header hdr =
			{
				.iVersionNo = 1,
				.unixtimeCreated = m_oMeta.unixtimeCreated
			};
			strcpy_s(hdr.szMagicNo, szMAGICNO);
			hdr.stridxFoundry = MAKESTRIDX(m_oMeta.sFoundry);
			hdr.stridxDesigner = MAKESTRIDX(m_oMeta.sDesigner);
			hdr.stridxComments = MAKESTRIDX(m_oMeta.sComments);

			iOffsetDynamicData = file.tellp() +
				((intptr_t)&hdr.iFilesize - (intptr_t)&hdr);
			WRITE(hdr);
		}

		// font family header
		{
			auto &oMeta = m_oFontFamily.getMetdata();
			FILE_FontFamilyHeader hdr =
			{
				.cFallback = oMeta.cFallback,
				.iTypefaceClass = static_cast<uint8_t>(oMeta.eType),
				.iFontCount = m_oFontFamily.getFonts().size()
			};
			hdr.stridxName = MAKESTRIDX(oMeta.sName);
			hdr.stridxCopyright = MAKESTRIDX(oMeta.sCopyright);

			WRITE(hdr);
		}

		// fonts
		{
			for (const auto &oFont : m_oFontFamily.getFonts())
			{
				// font header
				{
					const auto &oMeta = oFont.getMetadata();

					FILE_FontHeader hdr =
					{
						.cFallback = oMeta.cFallback,
						.iWeight = oMeta.iWeight,
						.iFaceCount = oFont.getFaces().size()
					};
					hdr.stridxName = MAKESTRIDX(oMeta.sName);
					hdr.stridxCopyright = MAKESTRIDX(oMeta.sCopyright);
					if (oMeta.bItalic)
						hdr.iFlags |= FontFlag_Italic;
					if (oMeta.bUnderline)
						hdr.iFlags |= FontFlag_Underline;
					if (oMeta.bStrikeout)
						hdr.iFlags |= FontFlag_Strikeout;

					WRITE(hdr);
				}

				// fontfaces
				{
					for (const auto &oFace : oFont.getFaces())
					{
						// header
						{
							const auto &oMeta = oFace.getMetadata();

							FILE_FontFaceHeader hdr =
							{
								.cFallback = oMeta.cFallback,
								.iXHeight = oMeta.iXHeight,
								.iCapHeight = oMeta.iCapHeight,
								.iPoints = oMeta.iPoints,
								.iCharCount = oFace.getCharacters().size()
							};
							hdr.stridxName = MAKESTRIDX(oMeta.sName);
							hdr.stridxCopyright = MAKESTRIDX(oMeta.sCopyright);

							WRITE(hdr);
						}



						FILE_CharMeta oCharMeta;

						// default character metadata
						{
							const auto &oDefCharMeta = oFace.getDefaultCharMetadata();
							oCharMeta =
							{
								.iWidth = oFace.getDefaultCharWidth(),
								.iHeight = oFace.getDefaultCharHeight(),
								.iBaseline = oDefCharMeta.iBaseline,
								.iPaddingLeft = oDefCharMeta.iPaddingLeft,
								.iPaddingRight = oDefCharMeta.iPaddingRight
							};
							WRITE(oCharMeta);
						}

						// character table
						for (const auto &it : oFace.getCharacters())
						{
							if (!it.second)
								continue; // don't save uninitialized characters

							WRITE(it.first);

							const auto &ch = it.second;
							const auto &chMeta = ch.getMetadata();

							oCharMeta =
							{
								.iWidth = ch.getWidth(),
								.iHeight = ch.getHeight(),
								.iBaseline = chMeta.iBaseline,
								.iPaddingLeft = chMeta.iPaddingLeft,
								.iPaddingRight = chMeta.iPaddingRight
							};
							WRITE(oCharMeta);
						}

						// graphics data
						for (const auto &it : oFace.getCharacters())
						{
							file.write((const char *)it.second.getData(),
								it.second.getDataSize());
						}

					}
				}

			}
		}

		// string table
		const auto iStringTableOffset = file.tellp();
		for (auto str : oStrings)
		{
			file.write(reinterpret_cast<const char *>(str->c_str()),
				(str->length() + 1) * sizeof(wchar_t));
		}



		// file-related metadata
		{
			uint64_t ui64 = file.tellp();
			file.seekp(iOffsetDynamicData);

			// write actual file size
			WRITE(ui64);

			// write actual string table offset
			ui64 = iStringTableOffset;
			WRITE(ui64);

			// write actual string count
			ui64 = oStrings.size();
			WRITE(ui64);
		}
	}
	catch (const std::exception &)
	{
		file.close();
		DeleteFileW(szFilename); // delete incomplete file
		return false;
	}
#undef WRITE
#undef WRITESTRIDX

	return true;
}

void rlFont::Project::setCreationUNIXTimestamp()
{
	m_oMeta.unixtimeCreated = UNIXTimestamp_Now();
}

bool rlFont::Project::loadFromFileV1(void *pIfstream, const void *pFileHeader)
{
	auto &file = *reinterpret_cast<std::ifstream *>(pIfstream);

	std::vector<std::wstring> oStrings;

	// file header, string table
	{
		const auto &hdr = *reinterpret_cast<const FILE_Header *>(pFileHeader);

		const auto offset = file.tellg();

		if (hdr.iOffsetStringtable > hdr.iFilesize)
			throw std::exception(); // invalid stringtable offset
		else if (hdr.iOffsetStringtable < hdr.iFilesize)
		{
			// read strings
			const size_t iSizeStringTable = hdr.iFilesize - hdr.iOffsetStringtable;

			auto up_strings =
				std::make_unique<wchar_t[]>(iSizeStringTable / sizeof(wchar_t));
			file.seekg(hdr.iOffsetStringtable, std::ios::beg);
			file.read(reinterpret_cast<char *>(up_strings.get()), iSizeStringTable);


			const wchar_t *sz = up_strings.get();
			for (size_t iString = 0; iString < hdr.iStringCount; ++iString)
			{
				const size_t len = std::wcslen(sz);

				oStrings.push_back(sz);

				sz += len + 1;
			}
		}

		file.seekg(offset); // jump back to after the header

		if (hdr.stridxFoundry)
			m_oMeta.sFoundry = oStrings[hdr.stridxFoundry - 1];
		if (hdr.stridxDesigner)
			m_oMeta.sDesigner = oStrings[hdr.stridxDesigner - 1];
		if (hdr.stridxComments)
			m_oMeta.sComments = oStrings[hdr.stridxComments - 1];
		m_oMeta.unixtimeCreated = hdr.unixtimeCreated;
	}

	// font family header
	size_t iFontCount;
	{
		FILE_FontFamilyHeader hdr{};
		file.read(reinterpret_cast<char *>(&hdr), sizeof(hdr));

		FontFamilyMeta oMeta;
		if (hdr.stridxName)
			oMeta.sName = oStrings[hdr.stridxName - 1];
		if (hdr.stridxCopyright)
			oMeta.sCopyright = oStrings[hdr.stridxCopyright - 1];
		hdr.cFallback = hdr.cFallback;
		switch (hdr.iTypefaceClass)
		{
		case 0:
			oMeta.eType = TypefaceClassification::DontCare;
			break;
		case 1:
			oMeta.eType = TypefaceClassification::Roman;
			break;
		case 2:
			oMeta.eType = TypefaceClassification::Swiss;
			break;
		case 3:
			oMeta.eType = TypefaceClassification::Modern;
			break;
		case 4:
			oMeta.eType = TypefaceClassification::Script;
			break;
		case 5:
			oMeta.eType = TypefaceClassification::Decorative;
			break;
		default:
			return false; // unknown font type
		}

		m_oFontFamily.setMetadata(oMeta);

		iFontCount = hdr.iFontCount;
	}

	// fonts
	for (size_t iFont = 0; iFont < iFontCount; ++iFont)
	{
		Font oFont;

		// font header
		size_t iFaceCount;
		{
			FILE_FontHeader hdr{};
			file.read(reinterpret_cast<char *>(&hdr), sizeof(hdr));

			FontMeta oMeta;
			if (hdr.stridxName)
				oMeta.sName = oStrings[hdr.stridxName - 1];
			if (hdr.stridxCopyright)
				oMeta.sCopyright = oStrings[hdr.stridxCopyright - 1];
			oMeta.cFallback  = hdr.cFallback;
			oMeta.iWeight    = hdr.iWeight;
			oMeta.bItalic    = hdr.iFlags & FontFlag_Italic;
			oMeta.bUnderline = hdr.iFlags & FontFlag_Underline;
			oMeta.bStrikeout = hdr.iFlags & FontFlag_Strikeout;

			iFaceCount = hdr.iFaceCount;

			oFont.setMetadata(oMeta);
		}

		// faces
		for (size_t iFace = 0; iFace < iFaceCount; ++iFace)
		{
			FontFace oFontFace;

			size_t iCharCount;

			// font face header
			{
				FILE_FontFaceHeader hdr{};
				file.read(reinterpret_cast<char *>(&hdr), sizeof(hdr));

				FontFaceMeta oMeta;

				if (hdr.stridxName)
					oMeta.sName = oStrings[hdr.stridxName - 1];
				if (hdr.stridxCopyright)
					oMeta.sCopyright = oStrings[hdr.stridxCopyright - 1];
				oMeta.cFallback  = hdr.cFallback;
				oMeta.iXHeight   = hdr.iXHeight;
				oMeta.iCapHeight = hdr.iCapHeight;
				oMeta.iPoints    = hdr.iPoints;

				oFontFace.setMetadata(oMeta);

				iCharCount = hdr.iCharCount;
			}

			// default character metadata
			{
				FILE_CharMeta oMeta{};
				file.read(reinterpret_cast<char *>(&oMeta), sizeof(oMeta));

				CharMeta oMeta2;
				oMeta2.iBaseline     = oMeta.iBaseline;
				oMeta2.iPaddingLeft  = oMeta.iPaddingLeft;
				oMeta2.iPaddingRight = oMeta.iPaddingRight;

				oFontFace.setDefaultCharMetadata(oMeta2);
				oFontFace.setDefaultCharWidth(oMeta.iWidth);
				oFontFace.setDefaultCharHeight(oMeta.iHeight);
			}

			// character table
			std::vector<char32_t> oChars;
			for (size_t iChar = 0; iChar < iCharCount; ++iChar)
			{
				FILE_CharTableEntry oEntry{};
				file.read(reinterpret_cast<char *>(&oEntry), sizeof(oEntry));

				auto &ch = oFontFace.getCharacters()[oEntry.ch];
				if (ch) // character is already valid
					return false; // character duplicate

				oChars.push_back(oEntry.ch);

				CharMeta oMeta;
				oMeta.iBaseline     = oEntry.oMetadata.iBaseline;
				oMeta.iPaddingLeft  = oEntry.oMetadata.iPaddingLeft;
				oMeta.iPaddingRight = oEntry.oMetadata.iPaddingRight;
				ch.setMetadata(oMeta);

				ch.create(oEntry.oMetadata.iWidth, oEntry.oMetadata.iHeight);
			}

			// graphics data
			for (size_t iChar = 0; iChar < iCharCount; ++iChar)
			{
				auto &oCh = oFontFace.getCharacters()[oChars[iChar]];
				const auto lenBuf = oCh.getDataSize();
				auto up_TMP = std::make_unique<uint8_t[]>(lenBuf);
				file.read(reinterpret_cast<char *>(up_TMP.get()), lenBuf);
				oCh.setData(up_TMP.get(), lenBuf);
			}

			oFont.getFaces().push_back(std::move(oFontFace));
		}

		m_oFontFamily.getFonts().push_back(std::move(oFont));
	}

	return true;

}
