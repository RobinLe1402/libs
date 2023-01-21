#include "rl/lib/RasterFontReader.hpp"

namespace lib = rl::RasterFontReader;

#include "include/DataReader.hpp"
#include "include/CodePageToUnicode.hpp"

#include <cstdint>
#include <vector>
#include <Windows.h>





//==================================================================================================
// RasterFontFaceCollection

lib::RasterFontFaceCollection::RasterFontFaceCollection(const RasterFontFaceCollection& other) :
	m_oFonts(other.m_oFonts) { }

lib::RasterFontFaceCollection::RasterFontFaceCollection(RasterFontFaceCollection&& rval) noexcept :
	m_oFonts(std::move(rval.m_oFonts)) {}

lib::LoadResult_CPI lib::RasterFontFaceCollection::loadFromFile_CPI(const wchar_t* szFilepath)
{
	clear();

	std::vector<uint8_t> oData;

	if (!LoadFileToMemory(szFilepath, oData))
		return lib::LoadResult_CPI::FileNotOpened;

	return loadFromData_CPI(&oData[0], oData.size());
}

lib::LoadResult_FON lib::RasterFontFaceCollection::loadFromFile_FON(const wchar_t* szFilepath,
	uint16_t iFallbackCodepage)
{
	clear();

	std::vector<uint8_t> oData;

	if (!LoadFileToMemory(szFilepath, oData))
		return lib::LoadResult_FON::FileNotOpened;

	return loadFromData_FON(&oData[0], oData.size(), iFallbackCodepage);
}

lib::LoadResult_CPI lib::RasterFontFaceCollection::loadFromData_CPI(const void* pData, size_t iSize)
{
	using result = lib::LoadResult_CPI;
	clear();

	DataReader reader(pData, iSize);
	reader.setExceptions(true);

	try
	{
#pragma pack(push, 1) // disable padding
		struct FontFileHeader
		{
			uint8_t id0;
			char id[7];
			uint8_t reserved[8];
			uint16_t pnum;
			uint8_t ptyp;
			uint32_t fih_offset;
		} filehdr{};
#pragma pack(pop)

		reader.readVar(filehdr);
		if (filehdr.id0 != 0xFF || memcmp(filehdr.id, "FONT   ", 7) != 0 || filehdr.pnum != 1 ||
			filehdr.ptyp != 1)
			return result::UnexpectedValue;

		reader.seekPos(filehdr.fih_offset);





		struct FontInfoHeader
		{
			uint16_t num_codepages;
		} fih{};
		reader.readVar(fih);

		if (fih.num_codepages == 0)
			return result::Success; // no codepages --> no error





#pragma pack(push, 1) // disable padding
		struct CodePageEntryHeader
		{
			uint16_t cpeh_size;
			uint32_t next_cpeh_offset;
			uint16_t device_type;
			char device_name[8];
			uint16_t codepage;
			uint8_t reserved[6];
			uint32_t cpih_offset;
		} cpeh{};
#pragma pack(pop)

		struct CodePageInfoHeader
		{
			uint16_t version;
			uint16_t num_fonts;
			uint16_t size;
		} cpih{};

		struct ScreenFontHeader
		{
			uint8_t height;
			uint8_t width;
			uint8_t yaspect;
			uint8_t xaspect;
			uint16_t num_chars;
		} screenhdr{};

		for (uint32_t iCP_ID = 0; iCP_ID < fih.num_codepages; ++iCP_ID)
		{
			reader.readVar(cpeh);
			if (cpeh.device_type == 1 && cpeh.cpeh_size == sizeof(cpeh))
			{
				reader.readVar(cpih);
				if (cpih.version <= 1) // "0 should be treated as 1"
				{
					for (uint32_t iFontID = 0; iFontID < cpih.num_fonts; ++iFontID)
					{
						reader.readVar(screenhdr);

						if (screenhdr.width != 8)
						{
							// skip font, as there is no universal interpretation of other widths
							reader.seekPos((size_t)screenhdr.num_chars * screenhdr.height *
								(((size_t)screenhdr.width + 7) / 8), DataSeekMethod::Current);
							continue;
						}


						m_oFonts.push_back(RasterFontFace());
						auto& face = m_oFonts.at(m_oFonts.size() - 1);
						auto& meta = face.meta();

						meta.iCodepage = cpeh.codepage;
						meta.iHeight = screenhdr.height;
						meta.iMaxWidth = screenhdr.width;
						meta.iMinWidth = screenhdr.width;
						meta.iMonoWidth = screenhdr.width;

						meta.sDeviceName.resize(8);
						memcpy_s(&meta.sDeviceName[0], meta.sDeviceName.length(),
							cpeh.device_name, 8);
						

						for (uint16_t iChar = 0; iChar < screenhdr.num_chars; ++iChar)
						{
							char32_t cRaw = 0;
							CodePageToUnicode(meta.iCodepage, (uint8_t)iChar, cRaw);

							face.chars().emplace(cRaw, RasterChar(8, meta.iHeight));
							auto& ch = face.chars().at(cRaw);
							
							for (size_t iY = 0; iY < meta.iHeight; ++iY)
							{
								uint8_t iRow = 0;
								reader.readVar(iRow);
								for (uint8_t iX = 0; iX < 8; ++iX)
								{
									ch.setPixel(iX, (unsigned int)iY, (iRow & 0x80));
									iRow <<= 1;
								}
							}
						}
					}
				}
			}
			if (iCP_ID < uint32_t(fih.num_codepages - 1))
				reader.seekPos(cpeh.next_cpeh_offset);
		}



		// read and set the copyright string located right after the last font
		if (!reader.eof())
		{
			const auto iPos = reader.tellPos();
			size_t iCopyrightLength = 0;
			uint8_t iChar = 0;
			do
			{
				reader.readVar(iChar);
				++iCopyrightLength;
			} while (!reader.eof() && iChar != 0 && iChar != 0x1A);
			if (iChar == 0 || iChar == 0x1A)
				--iCopyrightLength;

			std::string sCopyright;
			sCopyright.resize(iCopyrightLength);
			memcpy_s(&sCopyright[0], sCopyright.length(),
				reader.begin() + iPos, iCopyrightLength);

			for (auto& oFont : m_oFonts)
			{
				oFont.meta().sCopyright = sCopyright;
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



	return result::Success;
}

lib::LoadResult_FON lib::RasterFontFaceCollection::loadFromData_FON(const void* pData, size_t iSize,
	uint16_t iFallbackCodepage)
{
	using result = lib::LoadResult_FON;
	clear();

	DataReader reader(pData, iSize);
	reader.setExceptions(true);

	try
	{
		// jump to and read offset to segmented .EXE header
		reader.seekPos(0x3C); // jump to segmented .EXE header offset value
		WORD wOffset_NEHeader = 0;
		reader.readVar(wOffset_NEHeader);
		
		// jump to and read offset from segmented .EXE header to resource table
		reader.seekPos((size_t)wOffset_NEHeader + 0x24);
		WORD wOffset_ResourceTable = 0;
		reader.readVar(wOffset_ResourceTable);
		
		// jump to resource table
		reader.seekPos((size_t)wOffset_ResourceTable + wOffset_NEHeader);

		WORD wAlignShift = 0; // alignment shift count for resource data
		reader.readVar(wAlignShift);



		const size_t iResourcesStart = reader.tellPos();

		struct ResourceRec // per type
		{
			WORD wTypeID;
			WORD wCount;
			DWORD dwReserved;
		} rr{};
		
		struct ResourceTableEntry // per resource
		{
			WORD wOffset; // has to be shifted wAlignShift times
			WORD wLength;
			WORD wFlags;
			WORD wID;
			DWORD dwReserved;
		} rte{};



		// go through all resources, try to load FONT data
		while (reader.readVar(rr.wTypeID), rr.wTypeID != 0)
		{
			bool bRead = rr.wTypeID == 0x8008; // 0x8000 --> numeric ID; 8 --> font

			reader.readVar(rr.wCount);
			reader.readVar(rr.dwReserved);

			if (!bRead) // type is not of interest --> skip it's resources
			{
				reader.seekPos((size_t)rr.wCount * sizeof(ResourceTableEntry),
					DataSeekMethod::Current);
				continue;
			}

			for (uint32_t iRes = 0; iRes < rr.wCount; ++iRes)
			{
				reader.readVar(rte);

				RasterFontFace oFont;
				const auto result =
					oFont.loadFromData_FNT(
						reader.begin() + ((size_t)rte.wOffset << wAlignShift),
						(size_t)rte.wLength << wAlignShift, iFallbackCodepage);

				if (result == LoadResult_FNT::Success)
					m_oFonts.push_back(std::move(oFont));
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



	return result::Success;
}
