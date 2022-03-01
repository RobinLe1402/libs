#include "rl/graphics.fonts.bitmap.creator.hpp"

#include <rl/unicode.hpp>

#include <memory>
#include <Windows.h>





namespace rl
{

	/***********************************************************************************************
	 class BitmapFontChar
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	BitmapFontChar::BitmapFontChar(const BitmapFontChar& other)
	{
		*this = other;
	}

	BitmapFontChar::BitmapFontChar(BitmapFontChar&& rval) noexcept
	{
		*this = std::move(rval);
	}

	BitmapFontChar::BitmapFontChar(uint16_t Width, uint16_t Height, uint8_t BitsPerPixel,
		FontFaceBinaryFormat Format)
	{
		create(Width, Height, BitsPerPixel, Format);
	}

	BitmapFontChar::~BitmapFontChar()
	{
		destroy();
	}





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	BitmapFontChar& BitmapFontChar::operator=(const BitmapFontChar& other)
	{
		destroy();

		m_pData = new uint8_t[other.m_iDataSize];
		memcpy_s(m_pData, other.m_iDataSize, other.m_pData, other.m_iDataSize);

		m_iDataSize = other.m_iDataSize;
		m_iWidth = other.m_iWidth;
		m_iHeight = other.m_iHeight;
		m_iHeightRounded = other.m_iHeightRounded;
		m_iBPP = other.m_iBPP;
		m_eFormat = other.m_eFormat;


		return *this;
	}

	BitmapFontChar& BitmapFontChar::operator=(BitmapFontChar&& other) noexcept
	{
		destroy();

		m_pData = other.m_pData;
		m_iDataSize = other.m_iDataSize;
		m_iWidth = other.m_iWidth;
		m_iHeight = other.m_iHeight;
		m_iHeightRounded = other.m_iHeightRounded;
		m_iBPP = other.m_iBPP;
		m_eFormat = other.m_eFormat;

		other.m_pData = nullptr;
		other.m_iDataSize = 0;
		other.m_iWidth = 0;
		other.m_iHeight = 0;
		other.m_iHeightRounded = 0;
		other.m_iBPP = 0;
		other.m_eFormat = FontFaceBinaryFormat::BitPlanes;


		return *this;
	}





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	bool BitmapFontChar::create(uint16_t Width, uint16_t Height, uint8_t BitsPerPixel,
		FontFaceBinaryFormat Format)
	{
		if (Width == 0 || Height == 0 || BitsPerPixel == 0 || BitsPerPixel > 32 ||
			(Format == FontFaceBinaryFormat::RGB && BitsPerPixel != 3 * 8) ||
			(Format == FontFaceBinaryFormat::RGBA && BitsPerPixel != 4 * 8))
			return false;

		m_iWidth = Width;
		m_iHeight = Height;
		m_iHeightRounded = Height / 8;
		if (Height % 8)
			++m_iHeightRounded;
		m_iHeightRounded *= 8;

		m_iBPP = BitsPerPixel;
		m_eFormat = Format;


		switch (m_eFormat)
		{

		case FontFaceBinaryFormat::BitPlanes:
		{
			uint16_t iBytesPerCol = m_iHeight / 8;
			if (m_iHeight % 8)
				iBytesPerCol++;

			m_iDataSize = (size_t)iBytesPerCol * m_iWidth * m_iBPP;


			break;
		}

		case FontFaceBinaryFormat::FullBytes:
		{
			uint8_t iBytesPerPixel = m_iBPP / 8;
			if (m_iBPP % 8)
				iBytesPerPixel++;

			m_iDataSize = (size_t)iBytesPerPixel * m_iWidth * m_iHeight;


			break;
		}

		case FontFaceBinaryFormat::RGB:
		{
			m_iDataSize = (size_t)m_iWidth * m_iHeight * 3;
			break;
		}

		case FontFaceBinaryFormat::RGBA:
		{
			m_iDataSize = (size_t)m_iWidth * m_iHeight * 4;
			break;
		}


		default:
			throw std::exception("rl::BitmapFontChar: Unknown binary format");

		}



		m_pData = new uint8_t[m_iDataSize];
		memset(m_pData, 0, m_iDataSize); // initialize to zero


		return true;
	}

	void BitmapFontChar::destroy()
	{
		if (!created())
			return;

		delete[] m_pData;
		m_pData = nullptr;
		m_iDataSize = 0;
		m_iWidth = 0;
		m_iHeight = 0;
		m_iHeightRounded = 0;
		m_iBPP = 0;
		m_eFormat = FontFaceBinaryFormat::BitPlanes;
	}

	void BitmapFontChar::setPixel(uint16_t X, uint16_t Y, uint32_t Value)
	{
		checkData();
		checkPos(X, Y);


		switch (m_eFormat)
		{
		case FontFaceBinaryFormat::BitPlanes:
		{
			uint8_t* pData = m_pData + (intptr_t)X * ((intptr_t)m_iHeightRounded / 8 * m_iBPP) +
				Y / 8;
			const uint8_t iBitID = 7 - Y % 8;

			for (uint8_t i = 0; i < m_iBPP; ++i)
			{
				// 0 = least significant bit,
				// ...

				*pData &= ~(1 << (iBitID)); // unset old bit
				*pData |= ((Value >> i) & 1) << iBitID; // insert new bit

				pData += m_iHeightRounded / 8;
			}
		}
		break;


		case FontFaceBinaryFormat::FullBytes:
		{
			uint8_t iBytesBerPixel = m_iBPP / 8;
			if (m_iBPP % 8)
				++iBytesBerPixel;

			uint8_t* pData = m_pData + (Y + (intptr_t)X * m_iHeight) * iBytesBerPixel;

			for (uint8_t i = 0; i < iBytesBerPixel; ++i)
			{
				// 0 = least significant byte,
				// ...

				*pData = Value >> (i * 8);
				++pData;
			}
		}
		break;


		case FontFaceBinaryFormat::RGB:
		{
			uint8_t* pData = m_pData + (Y + (intptr_t)X * m_iHeight) * 3;

			for (uint8_t i = 0; i < 3; ++i)
			{
				// 0 = R
				// 1 = G
				// 2 = B

				*pData = (uint8_t)(Value >> (8 * (2 - i)));
				++pData;
			}
		}
		break;


		case FontFaceBinaryFormat::RGBA:
		{
			uint8_t* pData = m_pData + (Y + (intptr_t)X * m_iHeight) * 4;

			for (uint8_t i = 0; i < 4; ++i)
			{

				// 0 = R
				// 1 = G
				// 2 = B
				// 3 = A

				*pData = (uint8_t)(Value >> (8 * (3 - i)));
				++pData;
			}
		}

		}
	}

	uint32_t BitmapFontChar::getPixel(uint16_t X, uint16_t Y)
	{
		checkData();
		checkPos(X, Y);

		uint32_t iVal = 0;

		switch (m_eFormat)
		{

		case FontFaceBinaryFormat::BitPlanes:
		{
			const uint8_t* pData = m_pData + (intptr_t)X *
				((intptr_t)m_iHeightRounded / 8 * m_iBPP) + Y / 8;
			const uint8_t iBitID = 7 - Y % 8;

			for (uint8_t i = 0; i < m_iBPP; ++i)
			{
				// 0 = least significant bit,
				// ...

				iVal |= (((uint32_t)*pData >> (iBitID)) & 1) << i;

				pData += m_iHeightRounded / 8;
			}
		}
		break;

		case FontFaceBinaryFormat::FullBytes:
		{
			uint8_t iBytesBerPixel = m_iBPP / 8;
			if (m_iBPP % 8)
				++iBytesBerPixel;

			const uint8_t* pData = m_pData + (Y + (intptr_t)X * m_iHeight) * iBytesBerPixel;

			for (uint8_t i = 0; i < iBytesBerPixel; ++i)
			{
				// 0 = least significant byte,
				// ...

				iVal = (uint32_t)*pData << (i * 8);

				++pData;
			}
		}
		break;

		case FontFaceBinaryFormat::RGB:
		{
			const uint8_t* pData = m_pData + (Y + (intptr_t)X * m_iHeight) * 3;

			for (uint8_t i = 0; i < 3; ++i)
			{

				// 0 = R
				// 1 = G
				// 2 = B

				iVal |= (uint32_t)*pData << ((2 - i) * 8);

				++pData;
			}
		}
		break;

		case FontFaceBinaryFormat::RGBA:
		{
			const uint8_t* pData = m_pData + (Y + (intptr_t)X * m_iHeight) * 4;

			for (uint8_t i = 0; i < 4; ++i)
			{

				// 0 = R
				// 1 = G
				// 2 = B
				// 3 = A

				iVal |= (uint32_t)*pData << ((3 - i) * 8);

				++pData;
			}
		}
		break;

		}


		return iVal;
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	void BitmapFontChar::checkData()
	{
		if (!m_pData)
			throw std::exception("rl::BitmapFontChar: No data");
	}

	void BitmapFontChar::checkPos(uint16_t iX, uint16_t iY)
	{
		if (iX >= m_iWidth || iY >= m_iHeight)
			throw std::exception("rl::BitmapFontChar: Invalid position used");
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods










	/***********************************************************************************************
	 class BitmapFontFaceCreator
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
	// OPERATORS

	BitmapFontFaceCreator& BitmapFontFaceCreator::operator=(BitmapFontFaceCreator&& rval) noexcept
	{
		clear();


		m_bCreated = true;
		rval.m_bCreated = false;

		m_oChars.insert(std::make_move_iterator(rval.m_oChars.begin()),
			std::make_move_iterator(rval.m_oChars.end()));
		rval.m_oChars.clear();

		m_oConfig = rval.m_oConfig;
		rval.m_oConfig = {};

		m_sFontFamName = std::move(rval.m_sFontFamName);
		m_sFontFaceName = std::move(rval.m_sFontFaceName);
		m_sCopyright = std::move(rval.m_sCopyright);


		rval.clear();
		return *this;
	}

	BitmapFontFaceCreator& BitmapFontFaceCreator::operator=(const BitmapFontFaceCreator& other)
	{
		clear();
		if (!other.m_bCreated)
			return *this;

		m_bCreated = true;
		m_oChars = other.m_oChars;
		m_oConfig = other.m_oConfig;
		m_sFontFaceName = other.m_sFontFaceName;
		m_sFontFamName = other.m_sFontFamName;
		m_sCopyright = other.m_sCopyright;

		return *this;
	}

	BitmapFontFaceCreator& BitmapFontFaceCreator::operator=(const FontFaceClass& other)
	{
		clear();
		if (!other.hasData())
			return *this;


		m_eBinFormat = other.getBinaryFormat();
		m_iBitsPerPixel = other.getBitsPerPixel();
		m_iCharHeight = other.getHeight();
		m_iGlobalCharWidth = other.getFixedWidth();


		const uint16_t iFlags = other.getFlags();

		FontFaceCreatorConfig config = {};

		config.bEmoji = iFlags & RL_FNT_FLAG_EMOJI;
		config.bGenericUse = iFlags & RL_FNT_FLAG_GENERICUSE;
		config.bHighRes = iFlags & RL_FNT_FLAG_HIGHRES;
		config.bItalic = iFlags & RL_FNT_FLAG_ITALIC;
		config.bSymbols = iFlags & RL_FNT_FLAG_SYMBOLS;
		config.eClassification = static_cast<FontFaceClassification>(other.getClassification());
		other.getFaceVersion(config.iFaceVersion);
		config.iFallbackChar = other.getFallback();
		config.iPaddingFlags = other.getPaddingFlags();
		config.iWeight = other.getWeight();

		if (!create(config, other.getBinaryFormat(), other.getBitsPerPixel(), other.getHeight(),
			other.getFixedWidth(), other.getFamilyName(), other.getFaceName(),
			other.getCopyright()))
			throw std::exception("rl::BitmapFontFaceCreator: Couldn't create from FontFaceClass");

		for (const auto& o : other)
		{
			auto& ch = createChar(o.iCodepoint, o.iWidth);
			memcpy_s(ch.m_pData, ch.m_iDataSize,
				other.getData() + other.getCharInfo(o.iCodepoint)->iOffset, o.iSize);
		}

		return *this;
	}

	const BitmapFontChar& BitmapFontFaceCreator::operator[](char32_t ch) const
	{
		return getChar(ch);
	}





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	void BitmapFontFaceCreator::makeASCII(std::string& s)
	{
		for (char& ch : s)
		{
			if (ch > 0x7F)
				ch = '?';
		}
	}

	void BitmapFontFaceCreator::checkNoncharacter(char32_t ch)
	{
		if (Unicode::IsNoncharacter(ch))
			throw std::exception("rl::BitmapFontFaceCreator: Tried to work with a noncharacter "
				"value");
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	BitmapFontFaceCreator::BitmapFontFaceCreator(const BitmapFontFaceCreator& other)
	{
		*this = other;
	}

	BitmapFontFaceCreator::BitmapFontFaceCreator(BitmapFontFaceCreator&& other) noexcept
	{
		*this = std::move(other);
	}

	BitmapFontFaceCreator::BitmapFontFaceCreator(const FontFaceClass& other)
	{
		*this = other;
	}

	bool BitmapFontFaceCreator::create(const FontFaceCreatorConfig& Config,
		FontFaceBinaryFormat BinaryFormat, uint8_t BitsPerPixel, uint16_t CharHeight,
		uint16_t GlobalCharWidth, const char* szFontFamName, const char* szFontFaceName,
		const char* szCopyright)
	{
		clear();

		switch (BinaryFormat)
		{
		case FontFaceBinaryFormat::BitPlanes:
		case FontFaceBinaryFormat::FullBytes:
			if (BitsPerPixel == 0)
				return false;
			break;

		case FontFaceBinaryFormat::RGB:
			BitsPerPixel = 3;
			break;

		case FontFaceBinaryFormat::RGBA:
			BitsPerPixel = 4;
			break;
		}

		m_oConfig = Config;
		m_eBinFormat = BinaryFormat;
		m_iBitsPerPixel = BitsPerPixel;
		m_iCharHeight = CharHeight;
		m_iGlobalCharWidth = GlobalCharWidth;

		if (szFontFamName != nullptr)
			m_sFontFamName = szFontFamName;
		if (szFontFaceName != nullptr)
			m_sFontFaceName = szFontFaceName;
		if (szCopyright != nullptr)
			m_sCopyright = szCopyright;


		m_bCreated = true;
		return true;
	}

	void BitmapFontFaceCreator::clear()
	{
		m_bCreated = false;

		m_eBinFormat = FontFaceBinaryFormat::BitPlanes;
		m_iBitsPerPixel = 0;
		m_iCharHeight = 0;
		m_iGlobalCharWidth = 0;
		m_oChars.clear();
		m_oConfig = {};
		m_sFontFamName.clear();
		m_sFontFaceName.clear();
		m_sCopyright.clear();
	}

	void BitmapFontFaceCreator::setChar(char32_t ch, const BitmapFontChar& data)
	{
		checkNoncharacter(ch);

		m_oChars[ch] = data;
	}

	void BitmapFontFaceCreator::setChar(char32_t ch, BitmapFontChar&& data)
	{
		checkNoncharacter(ch);

		m_oChars[ch] = std::move(data);
	}

	BitmapFontChar& BitmapFontFaceCreator::createChar(char32_t ch, uint16_t width)
	{
		if (!m_bCreated)
			throw std::exception("rl::BitmapFontFaceCreator: Object was not ready for use");

		if (m_iGlobalCharWidth)
			width = m_iGlobalCharWidth;

		if (width == 0)
			throw std::exception("rl::BitmapFontFaceCreator: Tried to create zero-width character");

		if (Unicode::IsNoncharacter(ch))
			throw std::exception("rl::BitmapFontFaceCreator: Tried to create data for a "
				"noncharacter");

		BitmapFontChar o;

		o.create(width, m_iCharHeight, m_iBitsPerPixel, m_eBinFormat);

		m_oChars.emplace(ch, std::move(o));
		return m_oChars[ch];
	}

	const BitmapFontChar& BitmapFontFaceCreator::getChar(char32_t ch) const
	{
		const auto it = m_oChars.find(ch);
		if (it == m_oChars.end())
			throw std::exception("rl::BitmapFontFaceCreator: Requested undefined character data");

		return it->second;
	}

	void BitmapFontFaceCreator::setFontFamName(const char* szFontFamName)
	{
		if (!m_bCreated)
			return;

		m_sFontFamName = szFontFamName;
		makeASCII(m_sFontFamName);
	}

	void BitmapFontFaceCreator::setFontFaceName(const char* szFontFaceName)
	{
		if (!m_bCreated)
			return;

		m_sFontFaceName = szFontFaceName;
		makeASCII(m_sFontFaceName);
	}

	void BitmapFontFaceCreator::setCopyright(const char* szCopyright)
	{
		if (!m_bCreated)
			return;

		m_sCopyright = szCopyright;
		makeASCII(m_sCopyright);
	}

	bool BitmapFontFaceCreator::loadFromFile(const wchar_t* szFileName)
	{
		FontFaceClass oLoader;
		if (!oLoader.loadFromFile(szFileName))
			return false;

		*this = oLoader;
		return true;
	}

	bool BitmapFontFaceCreator::saveToFile(const wchar_t* szFileName) const
	{
		if (!m_bCreated || m_oChars.size() == 0)
			return false; // face was not initialized/there were no characters

		if (m_oChars.find(m_oConfig.iFallbackChar) == m_oChars.cend())
			return false; // fallback character was not defined


		FontFaceHeader hdr = {};

		const uint32_t iOffsetCharTable = sizeof(FontFaceHeader);
		const uint32_t iOffsetData = iOffsetCharTable +
			(uint32_t)m_oChars.size() * (uint32_t)sizeof(FontFaceCharInfo);
		uint32_t iOffsetStringTable = iOffsetData;
		for (auto& o : m_oChars)
			iOffsetStringTable += (uint32_t)o.second.getDataSize();

		// primary data

		hdr.iBinaryFormat = static_cast<uint8_t>(m_eBinFormat);
		hdr.iBitsPerPixel = m_iBitsPerPixel;
		hdr.iCharCount = (uint32_t)m_oChars.size();
		hdr.iCharHeight = m_iCharHeight;
		hdr.iClassification = static_cast<uint8_t>(m_oConfig.eClassification);

		hdr.iDataSize = iOffsetStringTable + uint32_t(m_sFontFamName.length() +
			m_sFontFaceName.length() + m_sCopyright.length()) + 3; // 3x terminating zero

		memcpy(hdr.iFaceVersion, m_oConfig.iFaceVersion, 4);
		hdr.iFallbackChar = m_oConfig.iFallbackChar;


		if (m_iGlobalCharWidth)
			hdr.iFlags |= RL_FNT_FLAG_MONOSPACED;
		if (m_oConfig.iPaddingFlags)
			hdr.iFlags |= RL_FNT_FLAG_PADDED;
		if (m_oConfig.bItalic)
			hdr.iFlags |= RL_FNT_FLAG_ITALIC;
		switch (m_eBinFormat)
		{
		case FontFaceBinaryFormat::RGBA:
			hdr.iFlags |= RL_FNT_FLAG_ALPHA;
			// no break; on purpose
		case FontFaceBinaryFormat::RGB:
			hdr.iFlags |= RL_FNT_FLAG_DIRECTCOLOR;
		}
		// search for non-ASCII characters
		for (auto& o : m_oChars)
		{
			// skip ASCII characters
			if (o.first <= 0x7F)
				continue;

			hdr.iFlags |= RL_FNT_FLAG_NONASCII;
			break;
		}
		if (m_oConfig.bSymbols | m_oConfig.bEmoji)
			hdr.iFlags |= RL_FNT_FLAG_SYMBOLS;
		if (m_oConfig.bEmoji)
			hdr.iFlags |= RL_FNT_FLAG_EMOJI;
		// search for characters in a private use area
		for (auto& o : m_oChars)
		{
			if (Unicode::IsPrivateUse(o.first))
			{
				hdr.iFlags |= RL_FNT_FLAG_PRIVATEUSEAREA;
				break;
			}
		}
		if (m_oConfig.bGenericUse)
			hdr.iFlags |= RL_FNT_FLAG_GENERICUSE;
		if (m_oConfig.bHighRes)
			hdr.iFlags |= RL_FNT_FLAG_HIGHRES;


		hdr.iGlobalCharWidth = m_iGlobalCharWidth;
		hdr.iOffsetCharTable = iOffsetCharTable;
		hdr.iOffsetFontFamName = iOffsetStringTable;
		hdr.iOffsetFontFaceName = hdr.iOffsetFontFamName + (uint32_t)m_sFontFamName.length() + 1;
		hdr.iOffsetCopyright = hdr.iOffsetFontFaceName + (uint32_t)m_sFontFaceName.length() + 1;
		hdr.iPaddingFlags = m_oConfig.iPaddingFlags & 0x0F;
		hdr.iWeight = m_oConfig.iWeight;




		// precalculated constants

		if (m_eBinFormat == FontFaceBinaryFormat::BitPlanes)
		{
			hdr.iFormatExtra = m_iCharHeight / 8;
			if (m_iCharHeight % 8)
				++hdr.iFormatExtra;

			hdr.iBytesPerColumn = m_iBitsPerPixel * hdr.iFormatExtra;
		}
		else
		{
			hdr.iFormatExtra = m_iBitsPerPixel / 8;
			if (m_iBitsPerPixel % 8)
				++hdr.iFormatExtra;

			hdr.iBytesPerColumn = m_iCharHeight * hdr.iFormatExtra;
		}






		HANDLE hFile = CreateFileW(szFileName, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false; // couldn't create file

		try
		{
			DWORD dwWritten = 0;
#define WRITE(p, len) \
			if (!WriteFile(hFile, p, len, &dwWritten, NULL) || dwWritten != len) \
				throw 0
#define WRITE_VAR(var) WRITE(&var, sizeof(var))
#define WRITE_STR(str) WRITE(str.c_str(), (DWORD)str.length() + 1)


			WRITE("rlFONTFACE", 10);
			uint8_t iFiletypeVer[2] = { 2, 0 };
			WRITE(&iFiletypeVer, 2);
			WRITE_VAR(hdr);

			FontFaceCharInfo chinfo = {};
			uint32_t iOffset = iOffsetData;
			for (auto& o : m_oChars)
			{
				chinfo.iCodepoint = o.first;
				chinfo.iOffset = iOffset;
				chinfo.iSize = (uint32_t)o.second.getDataSize();
				chinfo.iWidth = o.second.getWidth();

				WRITE_VAR(chinfo);

				iOffset += chinfo.iSize;
			}

			for (auto& o : m_oChars)
			{
				WRITE(o.second.getData(), (DWORD)o.second.getDataSize());
			}

			WRITE_STR(m_sFontFamName);
			WRITE_STR(m_sFontFaceName);
			WRITE_STR(m_sCopyright);


#undef WRITE_STR
#undef WRITE_VAR
#undef WRITE
		}
		catch (...)
		{
			CloseHandle(hFile);
			DeleteFileW(szFileName); // delete incomplete file
			return false;
		}

		CloseHandle(hFile);
		return true;
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
	// OPERATORS

	// operators





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