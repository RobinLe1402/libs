#include "rl/graphics.fonts.bitmap.reader.hpp"

#include <math.h>
#include <memory> // smart pointers
#include <stdint.h>
#include <Windows.h>





namespace rl
{

	bool FontFaceCreate(FontFace* dest, const uint8_t* buf, size_t cb, const uint8_t(&typever)[2])
	{
		if (cb > UINT32_MAX)
			return false;



#define MIN_VERSION "2.0"
		switch (typever[0])
		{
		case 2: // v2.X
		{
			switch (typever[1])
			{
			case 0: // v2.0
			{
				//----------------------------------------------------------------------------------
				// V2.0

				dest->pData = new uint8_t[cb];
				memcpy_s(dest->pData, cb, buf, cb);
				dest->pHeader = (FontFaceHeader*)dest->pData;
				dest->pInfos = (FontFaceCharInfo*)(dest->pData + sizeof(FontFaceHeader));

				return true;

				//----------------------------------------------------------------------------------
			}
			default:
				return false; // unknown minor version
			}
			break;
		}
		case 1: // v1.X -> v1.0 (only specified version)
			throw std::exception("rl::FontFaceClass: Tried loading a rlFNT v1.0 file. "
				"Please convert to newer version. Current minimum version is v" MIN_VERSION ".");
#undef MIN_VERSION
		default:
			return false; // unknown major version
		}
	}

	bool FontFaceCreateWithOverhead(FontFace* dest, const uint8_t* buf, size_t cb)
	{
		const char* szMagicNumber = "rlFONTFACE";
		const size_t lenMagicNumber = strlen(szMagicNumber);

		if (cb < lenMagicNumber + 2)
			return false; // buffer too small for magic number and type version

		if (memcmp(buf, szMagicNumber, lenMagicNumber) != 0)
			return false; // wrong magic number

		uint8_t iTypeVer[2];
		memcpy_s(iTypeVer, 2, buf + lenMagicNumber, 2);

		return FontFaceCreate(dest, buf + lenMagicNumber + 2, cb - lenMagicNumber - 2, iTypeVer);
	}

	void FontFaceCopy(FontFace* dest, const FontFace* src)
	{
		FontFaceFree(dest);

		const size_t size = src->pHeader->iDataSize;

		dest->pData = new uint8_t[size];
		memcpy_s(dest->pData, size, src->pData, size);
		dest->pHeader = (FontFaceHeader*)dest->pData;
		dest->pInfos = (FontFaceCharInfo*)(dest->pData + sizeof(FontFaceHeader));
	}

	void FontFaceFree(FontFace* data)
	{
		if (data->pData)
			delete[] data->pData;

		*data = {};
	}

	const FontFaceCharInfo* FontFaceFindChar(const FontFace* face, uint32_t ch)
	{
		for (size_t i = 0; i < face->pHeader->iCharCount; i++)
		{
			const FontFaceCharInfo* pInfos = face->pInfos + i;

			// it's assumed that all characters are sorted in ascending order

			if (pInfos->iCodepoint < ch)
				continue;
			if (pInfos->iCodepoint == ch)
				return pInfos;
			if (pInfos->iCodepoint > ch)
				return nullptr;
		}

		return nullptr;
	}

	// ToDo: Check for correct functionality
	bool FontFaceGetPixel(const FontFace* face, uint32_t ch, uint16_t x, uint16_t y,
		uint32_t* dest)
	{
		const FontFaceCharInfo* pInfo = FontFaceFindChar(face, ch);
		if (!pInfo)
			return false;
		const uint8_t* pData = face->pData + pInfo->iOffset;
		const auto iBPP = face->pHeader->iBitsPerPixel;
		*dest = 0;



		

		switch (static_cast<FontFaceBinaryFormat>(face->pHeader->iBinaryFormat))
		{

		case FontFaceBinaryFormat::BitPlanes:
		{
			const uint8_t* pCurrentByte =
				pData + ((size_t)x * face->pHeader->iBytesPerColumn) + (y / 8);
			const uint8_t iBitID = 7 - (y % 8);

			for (uint8_t i = 0; i < iBPP; ++i)
			{
				*dest |= (uint32_t)(((*pCurrentByte) >> iBitID) & 1) << i;

				pCurrentByte += face->pHeader->iFormatExtra;
			}

			break;
		}



		case FontFaceBinaryFormat::FullBytes:
		{
			for (uint8_t i = 0; i < iBPP; i += 8)
			{
				*dest |= (uint32_t)(pData[i / 8]) << i;
			}
			break;
		}



		case FontFaceBinaryFormat::RGB:
		{
			for (uint8_t i = 0; i < 3; ++i)
			{
				*dest |= (uint32_t)(pData[i]) << (2 - i);
			}
			break;
		}



		case FontFaceBinaryFormat::RGBA:
		{
			for (uint8_t i = 0; i < 4; ++i)
			{
				*dest |= (uint32_t)(pData[i]) << (2 - i);
			}
			break;
		}
			
		}


		
		return true;
	}





	/***********************************************************************************************
	 class FontFaceClass
	***********************************************************************************************/


	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	FontFaceClass::FontFaceClass(const FontFace& other)
	{
		if (!other.pData)
			return;

		FontFaceCopy(&m_oData, &other);
	}

	FontFaceClass::FontFaceClass(const FontFaceClass& other)
	{
		*this = other;
	}

	FontFaceClass::FontFaceClass(FontFaceClass&& rval) noexcept
	{
		*this = std::move(rval);
	}

	FontFaceClass::~FontFaceClass()
	{
		clear();
	}





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	FontFaceClass& FontFaceClass::operator=(const FontFaceClass& other)
	{
		if (this == &other)
			return *this;

		clear();
		if (other.hasData())
		{
			m_oData.pData = new uint8_t[other.m_oData.pHeader->iDataSize];
			m_oData.pHeader = (FontFaceHeader*)m_oData.pData;
			m_oData.pInfos = (FontFaceCharInfo*)(m_oData.pData + sizeof(FontFaceHeader));
		}

		return *this;
	}

	FontFaceClass& FontFaceClass::operator=(FontFaceClass&& rval) noexcept
	{
		if (this == &rval)
			return *this;

		clear();
		if (rval.hasData())
		{
			m_oData = rval.m_oData;
			rval.m_oData = {};
		}

		return *this;
	}

	FontFaceClass::const_iterator FontFaceClass::end() const
	{
		if (!m_oData.pData)
			return nullptr;

		return m_oData.pInfos + m_oData.pHeader->iCharCount;
	}





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void FontFaceClass::clear()
	{
		FontFaceFree(&m_oData);
	}

	bool FontFaceClass::loadFromDataWithOverhead(const uint8_t* buf, size_t size)
	{
		clear();
		return FontFaceCreateWithOverhead(&m_oData, buf, size);
	}

	bool FontFaceClass::loadFromData(const uint8_t* buf, size_t size, uint8_t(&typever)[2])
	{
		clear();
		return FontFaceCreate(&m_oData, buf, size, typever);
	}

	bool FontFaceClass::loadFromResource(HMODULE hModule, LPCSTR lpName)
	{
		clear();

		HRSRC hRsrc = FindResourceA(hModule, lpName, "FONTFACE");
		if (hRsrc == NULL)
			return false;

		HGLOBAL hGlobal = LoadResource(hModule, hRsrc);
		if (hGlobal == NULL)
			return false;

		const uint8_t* pData = (const uint8_t*)LockResource(hGlobal);
		if (pData == NULL)
			return false;

		DWORD dwResSize = SizeofResource(hModule, hRsrc);
		if (dwResSize == 0)
			return NULL;

		return FontFaceCreateWithOverhead(&m_oData, pData, dwResSize);
	}

	bool FontFaceClass::loadFromFile(const wchar_t* szFileName)
	{
		clear();

		HANDLE hFile = CreateFileW(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false; // couldn't open file for reading

		DWORD dwSizeHi = 0;
		DWORD dwSizeLo = GetFileSize(hFile, &dwSizeHi);
		if (dwSizeHi)
			return false; // file size must be 32Bit

		auto up_pData = std::make_unique<uint8_t[]>(dwSizeLo);
		DWORD dwRead = 0;
		if (!ReadFile(hFile, up_pData.get(), dwSizeLo, &dwRead, NULL) || dwRead != dwSizeLo)
		{
			CloseHandle(hFile);
			return false;
		}

		bool bResult = loadFromDataWithOverhead(up_pData.get(), dwSizeLo);
		CloseHandle(hFile);
		return bResult;
	}

	const char* FontFaceClass::getFamilyName() const
	{
		checkData();
		return (char*)(m_oData.pData + m_oData.pHeader->iOffsetFontFamName);
	}

	const char* FontFaceClass::getFaceName() const
	{
		checkData();
		return (char*)(m_oData.pData + m_oData.pHeader->iOffsetFontFaceName);
	}

	const char* FontFaceClass::getCopyright() const
	{
		checkData();
		return (char*)(m_oData.pData + m_oData.pHeader->iOffsetCopyright);
	}

	uint32_t FontFaceClass::getCharCount() const
	{
		checkData();
		return m_oData.pHeader->iCharCount;
	}

	char32_t FontFaceClass::getFallback() const
	{
		checkData();
		return m_oData.pHeader->iFallbackChar;
	}

	uint16_t FontFaceClass::getFixedWidth() const
	{
		checkData();
		return m_oData.pHeader->iGlobalCharWidth;
	}

	uint16_t FontFaceClass::getHeight() const
	{
		checkData();
		return m_oData.pHeader->iCharHeight;
	}

	uint16_t FontFaceClass::getWeight() const
	{
		checkData();
		return m_oData.pHeader->iWeight;
	}

	uint16_t FontFaceClass::getFlags() const
	{
		checkData();
		return m_oData.pHeader->iFlags;
	}

	uint8_t FontFaceClass::getBitsPerPixel() const
	{
		checkData();
		return m_oData.pHeader->iBitsPerPixel;
	}

	FontFaceBinaryFormat FontFaceClass::getBinaryFormat() const
	{
		checkData();
		return static_cast<FontFaceBinaryFormat>(m_oData.pHeader->iBinaryFormat);
	}

	uint8_t FontFaceClass::getPaddingFlags() const
	{
		checkData();
		return m_oData.pHeader->iPaddingFlags;
	}

	uint8_t FontFaceClass::getClassification() const
	{
		checkData();
		return m_oData.pHeader->iClassification;
	}

	void FontFaceClass::getFaceVersion(uint8_t(&dest)[4]) const
	{
		checkData();
		memcpy_s(dest, 4, m_oData.pHeader->iFaceVersion, 4);
	}

	const FontFace* FontFaceClass::getData() const
	{
		checkData();
		return &m_oData;
	}

	const FontFaceCharInfo* FontFaceClass::getCharInfo(char32_t codepoint) const
	{
		checkData();
		auto p = checkChar(codepoint);
		return p;
	}

	const FontFaceCharInfo* FontFaceClass::findChar(char32_t codepoint) const
	{
		checkData();
		return FontFaceFindChar(&m_oData, codepoint);
	}

	uint16_t FontFaceClass::getCharWidth(char32_t codepoint) const
	{
		checkData();
		auto p = checkChar(codepoint);
		return p->iWidth;
	}

	uint32_t FontFaceClass::getPixel(char32_t codepoint, uint16_t x, uint16_t y) const
	{
		checkData();
		uint32_t iResult = 0;

		if (!FontFaceGetPixel(&m_oData, codepoint, x, y, &iResult))
			throw std::exception("rl::FontFaceClass: The codepoint wasn't found");

		return iResult;
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void FontFaceClass::checkData() const
	{
		if (!m_oData.pData)
			throw std::exception("rl::FontFaceClass: The instance was empty");
	}

	const FontFaceCharInfo* FontFaceClass::checkChar(char32_t codepoint) const
	{
		auto p = findChar(codepoint);

		if (!p)
			throw std::exception("rl::FontFaceClass: The codepoint wasn't found");

		return p;
	}










	/***********************************************************************************************
	 class XXX
	***********************************************************************************************/


	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	// public methods





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods

}