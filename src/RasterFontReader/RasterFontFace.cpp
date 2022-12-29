#include "rl/lib/RasterFontReader.hpp"

namespace lib = rl::RasterFontReader;

#include "include/DataReader.hpp"
#include "include/CodePageToUnicode.hpp"

#include <exception>
#include <Windows.h>





//==================================================================================================
// HELP FUNCTIONS

constexpr uint16_t UnknownCodepage = 0xFFFF;

/// <summary>
/// Convert a FON character set identifier to a Windows codepage identifier
/// </summary>
/// <returns>Could a codepage be found?</returns>
bool CharSetToCodePage(uint8_t charset, uint16_t& codepage)
{
	switch (charset)
	{
#define CHARSET(ID, CP) case ID: codepage = CP; break

		CHARSET(ANSI_CHARSET, 1252);
		CHARSET(ARABIC_CHARSET, 1256);
		CHARSET(BALTIC_CHARSET, 1257);
		CHARSET(CHINESEBIG5_CHARSET, 950); // traditional chinese
		CHARSET(EASTEUROPE_CHARSET, 1250);
		CHARSET(GB2312_CHARSET, 936); // simple chinese
		CHARSET(GREEK_CHARSET, 1253);
		CHARSET(HANGEUL_CHARSET, 949);
		CHARSET(HEBREW_CHARSET, 1255);
		CHARSET(JOHAB_CHARSET, 1361);
		CHARSET(RUSSIAN_CHARSET, 1251);
		CHARSET(THAI_CHARSET, 874);
		CHARSET(SHIFTJIS_CHARSET, 932);
		CHARSET(TURKISH_CHARSET, 1254);

#undef CHARSET
	default:
		codepage = UnknownCodepage;
		return false;
	}
	return true;
}





//==================================================================================================
// HELP STRUCTS

#pragma pack(push, 1) // disable padding, as these structs are directly assigned binary data

/// <summary>
/// Base FNT header (for all versions)
/// </summary>
struct FONTHDR
{
	WORD  dfVersion;
	DWORD dfSize;
	CHAR  dfCopyright[60];
	WORD  dfType;
	WORD  dfPoints;
	WORD  dfVertRes;
	WORD  dfHorizRes;
	WORD  dfAscent;
	WORD  dfInternalLeading;
	WORD  dfExternalLeading;
	BYTE  dfItalic;
	BYTE  dfUnderline;
	BYTE  dfStrikeOut;
	WORD  dfWeight;
	BYTE  dfCharSet;
	WORD  dfPixWidth;
	WORD  dfPixHeight;
	BYTE  dfPitchAndFamily;
	WORD  dfAvgWidth;
	WORD  dfMaxWidth;
	BYTE  dfFirstChar;
	BYTE  dfLastChar;
	BYTE  dfDefaultChar;
	BYTE  dfBreakChar;
	WORD  dfWidthBytes;
	DWORD dfDevice;
	DWORD dfFace;
	DWORD dfReserved; // = dfBitsPointer

	/*
	CHAR  szDeviceName; // must be represented by a std::string or a char*
	CHAR  szFaceName; // must be represented by a std::string or a char*
	*/
};

#pragma pack(pop)





//==================================================================================================
// RasterFontFace

lib::RasterFontFace::RasterFontFace() : m_oMeta{} {}

lib::RasterFontFace::RasterFontFace(const RasterFontFace& other) :
	m_oChars(other.m_oChars), m_oMeta(other.m_oMeta) { }

lib::RasterFontFace::RasterFontFace(RasterFontFace&& rval) noexcept :
	m_oChars(std::move(rval.m_oChars)), m_oMeta(rval.m_oMeta) { }

lib::LoadResult_FNT lib::RasterFontFace::loadFromFile_FNT(const wchar_t* szFilepath,
	uint16_t iFallbackCodepage)
{
	std::vector<uint8_t> oData;

	if (!LoadFileToMemory(szFilepath, oData))
		return lib::LoadResult_FNT::FileNotOpened;

	return loadFromData_FNT(&oData[0], oData.size(), iFallbackCodepage);
}

lib::LoadResult_FNT lib::RasterFontFace::loadFromData_FNT(const void* pData, size_t iSize,
	uint16_t iFallbackCodepage)
{
	clear();

	DataReader reader(pData, iSize);
	reader.setExceptions(true);

	using result = lib::LoadResult_FNT;

	try
	{
		FONTHDR hdr{};
		reader.readVar(hdr);

		// vector font?
		if (hdr.dfType & 1)
			return result::VectorFont;
		// no codepage equivalent to the character set?
		if (!CharSetToCodePage(hdr.dfCharSet, m_oMeta.iCodepage))
		{
			if ((iFallbackCodepage == 0 || !IsValidCodePage(iFallbackCodepage)))
				return result::UnknownCharSet;

			m_oMeta.iCodepage = iFallbackCodepage;
		}

		// check the font version, read/skip extended header
		switch (hdr.dfVersion)
		{

			//--------------------------------------------------------------------------------------
			// FNT 2.0
			//
			// --> skip the following fields:
			//   DWORD dfBitsOffset;
			//   BYTE  dfReserved;
		case 0x0200:
			reader.seekPos(5, DataSeekMethod::Current);
			break;

			//--------------------------------------------------------------------------------------
			// FNT 3.0
			//
			// --> check if really 1bpp
		case 0x0300:
		{
#pragma pack (push, 1) // disable padding
			struct
			{
				DWORD dfBitsOffset;
				BYTE  dfReserved;
				DWORD dfFlags;
				WORD  dfAspace;
				WORD  dfBspace;
				WORD  dfCspace;
				DWORD dfColorPointer;
				BYTE  dfReserved1[16];
			} hdrEx{};
#pragma pack(pop)

			reader.readVar(hdrEx);

			if ((hdrEx.dfFlags & 0x0010) == 0) // 0x0010 = DFF_1COLOR
				return result::MultiColor;

			break;
		}

		default:
			return result::UnsupportedVersion;
		}



		m_oMeta.bItalic = hdr.dfItalic;
		m_oMeta.bStrikeOut = hdr.dfStrikeOut;
		m_oMeta.bUnderline = hdr.dfUnderline;
		m_oMeta.iAscent = hdr.dfAscent;
		m_oMeta.iHeight = hdr.dfPixHeight;
		m_oMeta.iPoints = hdr.dfPoints;
		m_oMeta.iWeight = hdr.dfWeight;
		m_oMeta.sFaceName = (const char*)reader.begin() + hdr.dfFace;
		m_oMeta.sDeviceName = (const char*)reader.begin() + hdr.dfDevice;
		m_oMeta.sCopyright = hdr.dfCopyright;
		if (!CodePageToUnicode(hdr.dfDefaultChar, m_oMeta.iCodepage, m_oMeta.cFallback))
			m_oMeta.cFallback = 0; // no reason to cancel loading the font; just set to "undefined".
		switch (hdr.dfPitchAndFamily)
		{
		case FF_ROMAN:
			m_oMeta.eType = FontType::Roman;
			break;
		case FF_SWISS:
			m_oMeta.eType = FontType::Swiss;
			break;
		case FF_MODERN:
			m_oMeta.eType = FontType::Modern;
			break;
		case FF_SCRIPT:
			m_oMeta.eType = FontType::Script;
			break;
		case FF_DECORATIVE:
			m_oMeta.eType = FontType::Decorative;
			break;
		case FF_DONTCARE: // fallthrough --> is default value
		default:
			m_oMeta.eType = FontType::DontCare;
			break;
		}
		// ignore hdr.dfPixWidth and hdr.dfMaxWidth as they might be inaccurate
		// --> we'll check this ourselves later, alongside m_oMeta.iMinWidth.


		WORD dfCharWidth; // always a WORD
		DWORD dfBitmapOffset; // WORD in FNT 2.0, DWORD in FNT 3.0
		WORD w;
		char32_t cRaw;

		for (uint16_t i = hdr.dfFirstChar; i <= hdr.dfLastChar; ++i)
		{
			reader.readVar(dfCharWidth);
			if (hdr.dfVersion == 0x0200)
			{
				reader.readVar(w);
				dfBitmapOffset = w;
			}
			else
				reader.readVar(dfBitmapOffset);

			const uint8_t* const pBase = reader.begin() + dfBitmapOffset;

			CodePageToUnicode((uint8_t)i, m_oMeta.iCodepage, cRaw);

			m_oChars.emplace(cRaw, RasterChar(dfCharWidth, hdr.dfPixHeight));
			auto& oChar = m_oChars.at(cRaw);


			if (oChar.width() <= 8)
			{
				for (size_t iY = 0; iY < oChar.height(); ++iY)
				{
					const uint8_t iRow = *(pBase + iY);

					for (uint8_t iX = 0; iX < oChar.width(); ++iX)
					{
						oChar.setPixel(iX, (unsigned)iY, (iRow >> (7 - iX)) & 1);
					}
				}
			}
			else // width > 8
			{
				// read all full columns
				const uint16_t iFullCols = oChar.width() / 8;
				for (unsigned iCol = 0; iCol < iFullCols; ++iCol)
				{
					const uint8_t* const pCol = pBase + (size_t)iCol * oChar.height();

					const unsigned iBaseX = iCol * 8;
					for (size_t iY = 0; iY < oChar.height(); ++iY)
					{
						const uint8_t iRow = *(pCol + iY);
						for (uint8_t iX = 0; iX < 8; ++iX)
						{
							oChar.setPixel(iBaseX + iX, (unsigned)iY, (iRow >> (7 - iX)) & 1);
						}
					}
				}

				// read incomplete column if available
				if (oChar.width() % 8)
				{
					const uint8_t* const pCol = pBase + (size_t)(iFullCols - 1) * oChar.height();
					const unsigned iBaseX = iFullCols * 8;
					for (size_t iY = 0; iY < oChar.height(); ++iY)
					{
						const uint8_t iRow = *(pCol + iY);

						for (uint8_t iX = 0; iX < oChar.width(); ++iX)
						{
							oChar.setPixel(iBaseX + iX, (unsigned)iY, (iRow >> (7 - iX)) & 1);
						}
					}
				}
			}
		}

	}
	catch (DataReaderException e)
	{
		(void)e; // just an indicator for the type of error

		clear();
		return result::UnexpectedEOF;
	}
	catch (...)
	{
		clear();
		return result::InternalError;
	}


	// get minimum and maximum width

	m_oMeta.iMaxWidth = 0;
	m_oMeta.iMinWidth = (unsigned int)-1;

	for (const auto& pair : m_oChars)
	{
		const auto iWidth = pair.second.width();

		if (iWidth < m_oMeta.iMinWidth)
			m_oMeta.iMinWidth = iWidth;
		if (iWidth > m_oMeta.iMaxWidth)
			m_oMeta.iMaxWidth = iWidth;
	}

	// minimum and maximum are identical --> monospaced!
	if (m_oMeta.iMinWidth == m_oMeta.iMaxWidth)
		m_oMeta.iMonoWidth = m_oMeta.iMinWidth;


	return result::Success;
}

void lib::RasterFontFace::clear()
{
	m_oChars.clear();
	m_oMeta = {};
}
